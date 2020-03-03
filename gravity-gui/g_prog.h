#ifndef G_PROG_H
#define G_PROG_H

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

#include <QObject>
#include <QWidget>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QProcessEnvironment>
#include <memory>
#include "ReplWidget.h"
#include "gravity_gui.h"

class GravityGui;

class GravityProg : public QObject
{
    Q_OBJECT

   public:
      explicit GravityProg(GravityGui *parent, ReplWidget *, QString);
      virtual ~GravityProg();
      bool progInvoke(QStringList list = QStringList());
      QProcess::ProcessState progIsRunning();
      void terminateProg() { process->terminate();}
      void setEnv(QStringList&);

   public slots:
      void progStarted();
      void progQuit(int, QProcess::ExitStatus);
      void stdIn(QString);
      void stdOut();
      void stdErr();

        // notify owner about interesting events
   signals:
      void progDone(int, QProcess::ExitStatus);
      void progStdOutText(QByteArray);

   private:
    unique_ptr<QProcess> process;
    QString clearStr;
    GravityGui *par;
    ReplWidget *terminal;
    QString program;
    void logToFile(const QString& msg);
};

#endif
