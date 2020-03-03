/*
Copyright 2005-2020 Kendall F. Morris

This file is part of the USF Gravity Gui software suite.

    The Gravity Gui suite is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation, either
    version 3 of the License, or (at your option) any later version.

    The suite is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the suite.  If not, see <https://www.gnu.org/licenses/>.
*/



// A utility class to support the common functions that all of the gravity
// console-based programs share.

#include "gravity_gui.h"
#include <iostream>
#include <QString>
#include <term.h>
#include <curses.h>
#include "g_prog.h"
#include <QDir>
#include <QFile>

using namespace std;

GravityProg::GravityProg(GravityGui* parent,ReplWidget* term, QString progName):par(parent),terminal(term),program(progName)
{
   process = make_unique<QProcess>(new QProcess(parent));

   if (qEnvironmentVariableIsEmpty("TERM")) // running from a shortcut?
   {
      QStringList env({"TERM","linux"});
      setEnv(env);
      setupterm("linux",1,0);  // grab clear screen escape seq progs will see
   }
   else
      setupterm(0,1,0);          // current term
   clearStr = tigetstr("clear"); // clear screen ESC code for selected terminal

   connect(terminal, &ReplWidget::command, this, [=](QString input) {stdIn(input);});
   connect(process.get(), &QProcess::readyReadStandardOutput, this, [=](){stdOut();});
   connect(process.get(), &QProcess::readyReadStandardError, this, [=](){stdErr();});
   connect(process.get(), &QProcess::started, this, [=](){progStarted();});
   connect(process.get(), static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [=](int code, QProcess::ExitStatus exit_status){progQuit(code,exit_status);}); 
}

// the connections will be disconnected when object is destroyed
GravityProg::~GravityProg()
{
   if (process && process->state() != QProcess::NotRunning)
   {
      process->kill();
      process->waitForFinished();
   }
}

QProcess::ProcessState GravityProg::progIsRunning()
{
   return process->state();
}

// run the program
bool GravityProg::progInvoke(QStringList args)
{
   bool running = true;
   if (process->state() == QProcess::NotRunning)  // just one instance
   {
      process->start(program,args);
      process->setTextModeEnabled(true);
      if (!process->waitForStarted(5000))
      {
         terminal->printWarn("\nProgram failed to start\n");
         running = false;
      }
   }
   return running;
}

void GravityProg::progStarted()
{
   terminal->clear();
   terminal->reset();
}

void GravityProg::progQuit(int code, QProcess::ExitStatus exit_status)
{
   QString msg;
   QTextStream outstat(&msg);
   outstat << endl << program.toLatin1().data() << " has exited.";
//   if (code !=0 || exit_status != 0)
//      outstat << " code: " << code << " exit status: " << exit_status;
   outstat << endl;
   terminal->append(msg);
   terminal->reset();
   if (process)
      emit progDone(code,exit_status);
}

// environment var(s) come in as entries in a string list, of form:
// [n] "name"
// [n+1] "value"
void GravityProg::setEnv(QStringList& vars)
{
   if (process)
   {
      QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
      for (auto iter = vars.constBegin(); iter != vars.constEnd(); )
      {
         env.insert(*iter,*(iter+1));
         iter += 2;
      }
      process->setProcessEnvironment(env);
   }
}

void GravityProg::stdErr()
{
   QByteArray prompts;
   string stuff;
   QString str;

   prompts = process->readAllStandardError();
   str = prompts;
   QString msg;
   QTextStream(&msg) << "\ngot error: " << str << endl;
   terminal->printWarn(msg);
}

void GravityProg::stdOut()
{
   QByteArray prompts;
   string stuff;
   QString str;

   prompts = process->readAllStandardOutput();
   int have_clear = prompts.indexOf(clearStr);   // handle clear screen esc code
   if (have_clear >= 0)
   {
      terminal->clearScreen();
      prompts.replace(clearStr,"");
   }
   str = prompts;
   terminal->result(str,false);
   progStdOutText(prompts);
//cout << "stdout: got from app" << endl;
//cout << str.toLatin1().data() << endl;
}

void GravityProg::stdIn(QString input)
{
//cout << "stdin [" << input.toLatin1().data() << "]" << endl;
   input += "\n";
   QByteArray charbytes = input.toLatin1();
   if (process)
      process->write(charbytes.data(),charbytes.length());
}

void GravityProg::logToFile(const QString& msg)
{
   QString outfile;
   QDir curdir("/home/dalexxxxx/work/usf/src/gravity-gui");
   if (curdir.exists())
      outfile="/home/dale/work/usf/src/gravity-gui/debug.out";
   else
      outfile="./debug.out";
   QFile logFile(outfile);
   if (logFile.open(QIODevice::WriteOnly | QIODevice::Append))
   {
      QTextStream out(&logFile);
      out << msg << endl;
      logFile.close();
   }
}




