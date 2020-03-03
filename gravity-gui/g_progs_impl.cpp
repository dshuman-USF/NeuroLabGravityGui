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



// Functions to manage the various programs gravity_gui runs.

#include <iostream>

#include <QMessageBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QFile>
#include <QDir>
#include <unistd.h>

#include "gravity_gui.h"
#include "ui_gravity_gui.h"
#include "g_prog.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"

//  GBATCH
// button click to invoke program
void GravityGui::doGbatch()
{
      // just one instance
   if (progGbatch && progGbatch->progIsRunning() != QProcess::NotRunning)
      return;

   gbatchSwitch();
   if (!haveGDT)
   {
      ui->gbatchTerm->printWarn("You have to load a .gdt file\n");
      ui->gbatchTerm->printWarn("or enter a base filename and save a param file\n");
      ui->gbatchTerm->printWarn("before you can run gbatch.");
      return;
   }

   if (selectedChans.size() < 2)
   {
      ui->gbatchTerm->printWarn("You have to select at least two neuron channel before you can run gbatch");
      return;
   }

   progGbatch = make_unique<GravityProg>(this,ui->gbatchTerm,"gbatch");
   connect(progGbatch.get(), static_cast<void(GravityProg::*)(int,QProcess::ExitStatus)>(&GravityProg::progDone), this, [=](int code,QProcess::ExitStatus exit_status){progGbatchDone(code,exit_status);}); 

    // gbatch crashes if the several files it will create already exist.
    // Either get permission to delete or or refuse to run gbatch.
   QString base = ui->baseName->text() + ui->fnameMod->text();
   QString gout = base+".gout";
   QString pos = base+".pos";
   QString dir = base+".dir";
   QFileInfo goutInfo(gout);
   QFileInfo posInfo(pos);
   QFileInfo dirInfo(dir);
   if (goutInfo.exists() || posInfo.exists() || dirInfo.exists())
   {
      QString msg;
      QTextStream(&msg) << tr("At least one these files:") << endl << gout << endl << pos << endl << dir << endl << tr("exists. The gbatch program will not run if these files exist.");
      QMessageBox msgBox(this);
      msgBox.setText(msg);
      msgBox.setInformativeText(tr("Do you want to delete these files before proceeding?"));
      msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
      msgBox.setDefaultButton(QMessageBox::Yes);
      msgBox.setIcon(QMessageBox::Question);
      int resp = msgBox.exec();
      if (resp != QMessageBox::Yes)
      {
         ui->gbatchTerm->printWarn("The gbatch program did not run.\n");
         return;    
      }
      else
      {
         QFile::remove(gout);
         QFile::remove(pos);
         QFile::remove(dir);
         msg.clear();
         QTextStream(&msg) << tr("The files ") << gout << " and " << pos << " and " << dir << tr(" hav been deleted.")<<endl;
         ui->gbatchTerm->append(msg);
      }
   }
   makeOffsetsGnew();  // if using a tuned option, this must exist before gbatch runs
   if (progGbatch->progInvoke())
   {
      ui->gBatch->setEnabled(false);
      ui->termTab->tabBar()->setTabTextColor(TABS::GBATCH,tabRunning);
      QString params = buildParams();
      progGbatch->stdIn(params);
   }
}

void GravityGui::progGbatchDone(int code,QProcess::ExitStatus exit_status)
{
   if (ui)
   {
      ui->termTab->tabBar()->setTabTextColor(TABS::GBATCH,tabBlack);
      ui->gBatch->setEnabled(true);
   }
}

// Make a defailt offsets.gnew file based on current params
// Assumes at least 2 neuron channels have been selected.
void GravityGui::makeOffsetsGnew()
{
   int num_sel = selectedChans.size();
   if (num_sel > 1)
   {
      int pair1, pair2;
      QFileInfo finfo("offsets.gnew");
      if (finfo.exists())
      {
         QString msg;
         QTextStream(&msg) << tr("The offsets.gnew file already exists.");
         QMessageBox msgBox(this);
         msgBox.setText(msg);
         msgBox.setInformativeText(tr("Do you want to over-write it?"));
         msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
         msgBox.setDefaultButton(QMessageBox::Yes);
         msgBox.setIcon(QMessageBox::Question);
         int resp = msgBox.exec();
         if (resp != QMessageBox::Yes)
         return;    
      }

      QFile file("offsets.gnew");
      QTextStream text(&file);
      if (!file.open(QIODevice::WriteOnly))
      {
         QString msg;
         QTextStream(&msg) << tr("Error opening file offsets.gnew ") << endl << tr("Error is:               ") << file.errorString();
         ui->gbatchTerm->printWarn(msg);
      }
      else
      {
         for (pair1 = 2; pair1 <= num_sel; ++pair1)
            for (pair2 = 1; pair2 < pair1; ++pair2)
               text << pair1 << ", " << pair2 << ", " << "0" << endl;
         file.close();
      }
   }
}


// XTRYDIS
void GravityGui::doXtrydis()
{
   if (progTrydis && progTrydis->progIsRunning() != QProcess::NotRunning)  // just one instance
      return;
   progTrydis = make_unique<GravityProg>(this,ui->xtrydisTerm,"xtrydis");
   connect(progTrydis.get(), static_cast<void(GravityProg::*)(int,QProcess::ExitStatus)>(&GravityProg::progDone), this, [=](int code,QProcess::ExitStatus exit_status){progXtrydisDone(code,exit_status);}); 
   connect(progTrydis.get(), static_cast<void(GravityProg::*)(QByteArray)>(&GravityProg::progStdOutText), this, [=](QByteArray text){progXtrydisGotLine(text);}); 
   ui->xtrydisTerm->setPrompt("");

   xtrydisSwitch();
   if (progTrydis->progInvoke())
   {
      ui->termTab->tabBar()->setTabTextColor(TABS::XTRYDIS,tabRunning);
      ui->xtrydisButton->setEnabled(false);
   }
}

void GravityGui::progXtrydisDone(int code,QProcess::ExitStatus exit_status)
{
   if (ui)
   {
      ui->xtrydisButton->setEnabled(true);
      ui->termTab->tabBar()->setTabTextColor(TABS::XTRYDIS,tabBlack);
   }
}

void GravityGui::progXtrydisGotLine(QByteArray stuff)
{
   QString fName;
   QByteArray charbytes;

   ui->xtrydisTerm->activateWindow();
   ui->xtrydisTerm->setFocus();
     // The first prompt from xtrydis is for the .pos file. As a one-time
     // thing, use the current base name and insert the .pos file.
     // This breaks if the prompt text changes.
   if (stuff.contains("input file"))
   {
      if (ui->filePrompt->isChecked())
        fName = QFileDialog::getOpenFileName(this,
                      tr("Select .pos file."), "./", ".pos Files (*.pos)");

      if (!fName.length())
         fName = ui->baseName->text() + ui->fnameMod->text() + ".pos";
      else
         fName = QFileInfo(fName).fileName();
      
      charbytes = fName.toLatin1();
      ui->xtrydisTerm->defaultResponse(charbytes);
   }
}

//  XPROJTM
void GravityGui::doXprojtm()
{
   if (progXprojtm && progXprojtm->progIsRunning() != QProcess::NotRunning)
      return;
   progXprojtm = make_unique<GravityProg>(this,ui->xprojtmTerm,"xprojtm");
   connect(progXprojtm.get(), static_cast<void(GravityProg::*)(int,QProcess::ExitStatus)>(&GravityProg::progDone), this, [=](int code,QProcess::ExitStatus exit_status){progXprojtmDone(code,exit_status);}); 
   connect(progXprojtm.get(), static_cast<void(GravityProg::*)(QByteArray)>(&GravityProg::progStdOutText), this, [=](QByteArray text){progXprojtmGotLine(text);}); 

   xprojtmSwitch();
   if (progXprojtm->progInvoke())
   {
      ui->termTab->tabBar()->setTabTextColor(TABS::XPROJTM,tabRunning);
      ui->xprojtmButton->setEnabled(false);
   }
}

void GravityGui::progXprojtmDone(int code,QProcess::ExitStatus exit_status)
{
   if (ui)
   {
      ui->termTab->tabBar()->setTabTextColor(TABS::XPROJTM,tabBlack);
      ui->xprojtmButton->setEnabled(true);
   }
}

void GravityGui::progXprojtmGotLine(QByteArray stuff)
{
   QString fName;
   QByteArray charbytes;

   ui->xprojtmTerm->activateWindow();
   ui->xprojtmTerm->setFocus();
  
     // This breaks if the prompt text changes.
   if (stuff.contains("POSITION FILE NAME"))
   {
      if (ui->filePrompt->isChecked())
        fName = QFileDialog::getOpenFileName(this,
                      tr("Select .gout file."), "./", ".gout Files (*.gout)");

      if (!fName.length())
         fName = ui->baseName->text() + ui->fnameMod->text() + ".gout";
      else
         fName = QFileInfo(fName).fileName();

      charbytes = fName.toLatin1();
      ui->xprojtmTerm->defaultResponse(charbytes);
   }
}

// EDT_SURROGATES
void GravityGui::doSurrogates()
{
   if (progSurrogates && progSurrogates->progIsRunning() != QProcess::NotRunning)  // just one instance
      return;

   progSurrogates = make_unique<GravityProg>(this,ui->surrogatesTerm,"edt_surrogate");
   connect(progSurrogates.get(), static_cast<void(GravityProg::*)(int,QProcess::ExitStatus)>(&GravityProg::progDone), this, [=](int code,QProcess::ExitStatus exit_status){progSurrogatesDone(code,exit_status);}); 

   surrogatesSwitch();
   QStringList args;
   if (setSurrogatesArgs(args))
      if (progSurrogates->progInvoke(args))
      {
         ui->termTab->tabBar()->setTabTextColor(TABS::SURROGATES,tabRunning);
         ui->surrogatesButton->setEnabled(false);
         ui->gsigButton->setEnabled(false);
      }
}

void GravityGui::progSurrogatesDone(int code,QProcess::ExitStatus exit_status)
{
   if (ui)
   {
      ui->termTab->tabBar()->setTabTextColor(TABS::SURROGATES,tabBlack);
      ui->surrogatesButton->setEnabled(true);
      ui->gsigButton->setEnabled(true);
   }
}


// GSIG
// shares terminal with SURROGATES
void GravityGui::doGsig()
{
   if (progGsig && progGsig->progIsRunning() != QProcess::NotRunning)
      return;

      // share terminal with surrogates term
   progGsig = make_unique<GravityProg>(this,ui->surrogatesTerm,"gsig");
   connect(progGsig.get(), static_cast<void(GravityProg::*)(int,QProcess::ExitStatus)>(&GravityProg::progDone), this, [=](int code,QProcess::ExitStatus exit_status){progGsigDone(code,exit_status);}); 

   gsigSwitch();
    // gsig (which is gbatch running under a different name)
    // crashes if it's output file exist from a previous run.
    // Either get permission to delete or or refuse to run gsig.
   QDir dir("./");
   dir.setNameFilters(QStringList() << "sh*.gout" << "sh*.pos" << "sh*.dir");
   dir.setFilter(QDir::Files);
   if (!dir.entryList().isEmpty())
   {
      QString msg;
      QTextStream(&msg) << tr("Files from a previous gsig run exist. The gsig program will not run if these files exist.");
      QMessageBox msgBox(this);
      msgBox.setText(msg);
      msgBox.setInformativeText(tr("Do you want to delete these files before proceeding?"));
      msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
      msgBox.setDefaultButton(QMessageBox::Yes);
      msgBox.setIcon(QMessageBox::Question);
      int resp = msgBox.exec();
      if (resp != QMessageBox::Yes)
      {
         ui->surrogatesTerm->printWarn("The gsig program did not run.\n");
         return;    
      }
      else
      {
         foreach(QString dirFile, dir.entryList())
            dir.remove(dirFile);

         msg.clear();
         QTextStream(&msg) << tr("The files ") << "sh*.gout, " << "sh*.pos, " << " and sh*.dir " << tr(" have been deleted.")<<endl;
         ui->surrogatesTerm->append(msg);
      }
   }

   if (progGsig->progInvoke())
   {
      ui->termTab->tabBar()->setTabTextColor(TABS::SURROGATES,tabRunning);
      ui->surrogatesButton->setEnabled(false);
      ui->gsigButton->setEnabled(false);
      QString params = buildParams();
      progGsig->stdIn(params);
   }
}

void GravityGui::progGsigDone(int code,QProcess::ExitStatus exit_status)
{
   if (ui)
   {
      ui->termTab->tabBar()->setTabTextColor(TABS::SURROGATES,tabBlack);
      ui->gsigButton->setEnabled(true);
      ui->surrogatesButton->setEnabled(true);
   }
}


// XSLOPE
void GravityGui::doXslope()
{
   if (progXslope && progXslope->progIsRunning() != QProcess::NotRunning)  // just one instance
      return;
   progXslope = make_unique<GravityProg>(this,ui->xslopeTerm,"xslope");
   connect(progXslope.get(), static_cast<void(GravityProg::*)(int,QProcess::ExitStatus)>(&GravityProg::progDone), this, [=](int code,QProcess::ExitStatus exit_status){progXslopeDone(code,exit_status);}); 
   connect(progXslope.get(), static_cast<void(GravityProg::*)(QByteArray)>(&GravityProg::progStdOutText), this, [=](QByteArray text){progXslopeGotLine(text);}); 


   xslopeSwitch();

   if (progXslope->progInvoke())
   {
      ui->termTab->tabBar()->setTabTextColor(TABS::XSLOPE,tabRunning);
      ui->xslopeButton->setEnabled(false);
      waitForWinTime=1;
   }
}

void GravityGui::progXslopeDone(int code,QProcess::ExitStatus exit_status)
{
   if (ui)
   {
      ui->termTab->tabBar()->setTabTextColor(TABS::XSLOPE,tabBlack);
      ui->xslopeButton->setEnabled(true);
   }
}

void GravityGui::progXslopeGotLine(QByteArray stuff)
{
   QString fName;
   QByteArray charbytes;

     // This breaks if the prompt text changes.
   if (stuff.contains("input file"))
   {
      if (ui->filePrompt->isChecked())
        fName = QFileDialog::getOpenFileName(this,
                      tr("Select .pos file."), "./", ".pos Files (*.pos)");
      if (!fName.length())
         fName = ui->baseName->text() + ui->fnameMod->text() + ".pos";
      else
         fName = QFileInfo(fName).fileName();
      charbytes = fName.toLatin1();
      ui->xslopeTerm->defaultResponse(charbytes);
   }
   else if (stuff.contains("CHOOSE DATA"))
   {
      sleep(waitForWinTime);  // 1st time, let xslope window be drawn and 
      waitForWinTime=0;       // be given focus, then we'll steal it back
                              // this works most of the time
   }
   else if (stuff.contains("Reading in control"))
   {
      return; // This can take a long time, so let user 
              // do other things, don't steal focus.
   }
   ui->xslopeTerm->activateWindow();
   ui->xslopeTerm->setFocus();
}


// SPK family of functions. Only one of these can run at a time.
// The button event handler passes the program name.
void GravityGui::doSpkPat(QString progname)
{
      // just one instance
   if (progSpkPat && progSpkPat->progIsRunning() != QProcess::NotRunning)
      return;
   progSpkPat = make_unique<GravityProg>(this,ui->spkPatTerm,progname);
   connect(progSpkPat.get(), static_cast<void(GravityProg::*)(int,QProcess::ExitStatus)>(&GravityProg::progDone), this, [=](int code,QProcess::ExitStatus exit_status){progSpkPatDone(code,exit_status);}); 
   connect(progSpkPat.get(), static_cast<void(GravityProg::*)(QByteArray)>(&GravityProg::progStdOutText), this, [=](QByteArray text){progSpkPatGotLine(text);}); 

   QStringList env({"newsur","1"});
   progSpkPat.get()->setEnv(env);

   spkPatSwitch();

    // The spkpat programs crashes if their output files exist from a previous
    // run.  Either get permission to delete or or refuse to run.
   QDir dir("./");
   dir.setNameFilters(QStringList() << "*.spk" << "*.txt" << "*.fwk");
   dir.setFilter(QDir::Files);
   if (!dir.entryList().isEmpty())
   {
      QString msg;
      QTextStream(&msg) << tr("Files from a previous run of one of the spkpat programs exist. This  program will not run if these files exist.");
      QMessageBox msgBox(this);
      msgBox.setText(msg);
      msgBox.setInformativeText(tr("Do you want to delete these files before proceeding?"));
      msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
      msgBox.setDefaultButton(QMessageBox::Yes);
      msgBox.setIcon(QMessageBox::Question);
      int resp = msgBox.exec();
      if (resp != QMessageBox::Yes)
      {
         msg.clear();
         QTextStream(&msg) << tr("The program ") << progname << tr(" did not run.\n"); 
         ui->spkPatTerm->printWarn(msg);
         return;    
      }
      else
      {
         foreach(QString dirFile, dir.entryList())
            dir.remove(dirFile);

         msg.clear();
         QTextStream(&msg) << tr("The files ") << "*.spk, " << "*.txt, " << " and *.fwk" << tr(" have been deleted.")<<endl;
         ui->spkPatTerm->append(msg);
      }
   }

   if (progSpkPat->progInvoke())
   {
      // disable all of the buttons.
      ui->termTab->tabBar()->setTabTextColor(TABS::SPKPAT,tabRunning);
      ui->spkpat6bg->setEnabled(false);
      ui->spkpat6bgr->setEnabled(false);
      ui->spkpat6kbg->setEnabled(false);
      ui->spkpatdist->setEnabled(false);
      ui->spkpatwip->setEnabled(false);
   }
}


// Fixed file name by default, but could be others.
// todo check for prompt for files
void GravityGui::progSpkPatGotLine(QByteArray stuff)
{
   QString fName, saveFName;
   QByteArray charbytes;

   if (stuff.contains("Input data file name"))
   {
      if (ui->filePrompt->isChecked())
        fName = QFileDialog::getOpenFileName(this,
                      tr("Select .out file."), "idlmov.out", ".out Files (*.out)");
      if (!fName.length())
         fName = "idlmov.out";
      else
         fName = QFileInfo(fName).fileName();
      charbytes = fName.toLatin1();
      ui->spkPatTerm->defaultResponse(charbytes);
   }
   else if (stuff.contains("Output data file name"))
   {
      saveFName = ui->baseName->text() + ui->fnameMod->text() + ".pat";
      if (ui->filePrompt->isChecked())
        fName = QFileDialog::getSaveFileName(this,
                      tr("Save .pat file."), saveFName, ".pat Files (*.pat)");
      if (!fName.length())
         fName = saveFName;
      else
         fName = QFileInfo(fName).fileName();
      charbytes = fName.toLatin1();
      ui->spkPatTerm->defaultResponse(fName);
      QFileInfo finfo(fName);
      if (finfo.exists())
      {
         QMessageBox msgBox(this);
         QString msg;
         QTextStream(&msg) << tr("The file: ") << fName << tr(" exists.") << endl << tr(" The spkpat family of programs will not run if this files exist.");
         msgBox.setText(msg);
         msgBox.setInformativeText(tr("Do you want to delete this file before proceeding?"));
         msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
         msgBox.setDefaultButton(QMessageBox::Yes);
         msgBox.setIcon(QMessageBox::Question);
         int resp = msgBox.exec();
         if (resp != QMessageBox::Yes)
         {
            progSpkPat->terminateProg();
            return;
         }
         else
         {
            QFile::remove(fName);
         }
      }
   }
   else if (stuff.contains("control cycles done"))
   {
      return; // This can take a long time, so let user 
              // do other things, don't steal focus.
   }
   ui->spkPatTerm->activateWindow();
   ui->spkPatTerm->setFocus();
}


void GravityGui::progSpkPatDone(int code,QProcess::ExitStatus exit_status)
{
   if (ui)
   {
      ui->termTab->tabBar()->setTabTextColor(TABS::SPKPAT,tabBlack);
      ui->spkpat6bg->setEnabled(true);
      ui->spkpat6bgr->setEnabled(true);
      ui->spkpat6kbg->setEnabled(true);
      ui->spkpatdist->setEnabled(true);
      ui->spkpatwip->setEnabled(true);
   }
}


// FIREWORKS
void GravityGui::doFireworks()
{
      // just one instance
   if (progFireworks && progFireworks->progIsRunning() != QProcess::NotRunning)
      return;

   progFireworks = make_unique<GravityProg>(this,ui->fireworksTerm,"fireworks");
   connect(progFireworks.get(), static_cast<void(GravityProg::*)(int,QProcess::ExitStatus)>(&GravityProg::progDone), this, [=](int code,QProcess::ExitStatus exit_status){progFireworksDone(code,exit_status);});
   connect(progFireworks.get(), static_cast<void(GravityProg::*)(QByteArray)>(&GravityProg::progStdOutText), this, [=](QByteArray text){progFireworksGotLine(text);});

   fireworksSwitch();

   if (progFireworks->progInvoke())
   {
      ui->termTab->tabBar()->setTabTextColor(TABS::FIREWORKS,tabRunning);
      ui->fireworksButton->setEnabled(false);
      waitForWinTime=1;
   }
}

void GravityGui::progFireworksGotLine(QByteArray stuff)
{
   QString fName, saveFName;
   QByteArray charbytes;

   ui->fireworksTerm->activateWindow();
   ui->fireworksTerm->setFocus();
   if (stuff.contains("Enter fireworks filename"))
   {
      // no obvious default file name and can be many to choose from,
      // so always prompt for file
     fName = QFileDialog::getOpenFileName(this,
                   tr("Select .fwk file."), "./", ".fwk Files (*.fwk)");
      if (fName.length())  // note: no obvious default file name
      {
         fName = QFileInfo(fName).fileName();
         charbytes = fName.toLatin1();
         ui->fireworksTerm->defaultResponse(charbytes);
      }
      else  // no fname, no fireworks
      {
         progFireworks->terminateProg();
      }
   }
   else if (stuff.contains("BDT File for analog"))
   {
      if (analogList.size()) // if no analog, no need for bdt file
      {
           // there is no easy way to know what the answer to this prompt is,
           // so assume they want one. If not, hit cancel. 
        fName = QFileDialog::getOpenFileName(this,
                      tr("Select .bdt file or Cancel to skip."), "./", ".bdt Files (*.bdt)");
         if (fName.length())
         {
            fName = QFileInfo(fName).fileName();
            charbytes = fName.toLatin1();
            ui->fireworksTerm->defaultResponse(charbytes);
         }
      }
      else
      {
         ui->fireworksTerm->fakeEnter();
      }
   }
   else if (stuff.contains("next page"))
   {
      sleep(waitForWinTime);  // 1st time, let xslope window be drawn and 
      waitForWinTime=0;       // be given focus, then we'll steal it back
                              // this works most of the time
   }

   ui->fireworksTerm->activateWindow();
   ui->fireworksTerm->setFocus();
}

void GravityGui::progFireworksDone(int code,QProcess::ExitStatus exit_status)
{
   if (ui)
   {
      ui->termTab->tabBar()->setTabTextColor(TABS::FIREWORKS,tabBlack);
      ui->fireworksButton->setEnabled(true);
   }
}


// 3DJMP
void GravityGui::do3DJmp()
{
      // just one instance
   if (prog3DJmp && prog3DJmp->progIsRunning() != QProcess::NotRunning)
      return;

   prog3DJmp = make_unique<GravityProg>(this,ui->threeDJmpTerm,"3djmp");
   connect(prog3DJmp.get(), static_cast<void(GravityProg::*)(int,QProcess::ExitStatus)>(&GravityProg::progDone), this, [=](int code,QProcess::ExitStatus exit_status){prog3DJmpDone(code,exit_status);});
   connect(prog3DJmp.get(), static_cast<void(GravityProg::*)(QByteArray)>(&GravityProg::progStdOutText), this, [=](QByteArray text){prog3DJmpGotLine(text);});

   threeDJmpSwitch();

   if (prog3DJmp->progInvoke())
   {
      ui->termTab->tabBar()->setTabTextColor(TABS::THREEDJMP,tabRunning);
      ui->threeDJmpButton->setEnabled(false);
   }
}

void GravityGui::prog3DJmpGotLine(QByteArray stuff)
{
   QString fName;
   QByteArray charbytes;

   if (stuff.contains("Input data file"))
   {
      // no obvious default file name and can be many to choose from,
      // so always prompt for file
     fName = QFileDialog::getOpenFileName(this,
                   tr("Select .spk .out file."), "./", ".spk and .out Files (*.spk *.out)");
      if (fName.length())
      {
         fName = QFileInfo(fName).fileName();
         charbytes = fName.toLatin1();
         ui->threeDJmpTerm->defaultResponse(charbytes);
      }
   }
   else if (stuff.contains("BDT File"))
   {
      if (analogList.size()) // if no analog, no need for bdt file
      {
           // there is no easy way to know what the answer to this prompt is,
           // so assume they want one. If not, hit cancel. 
        fName = QFileDialog::getOpenFileName(this,
                      tr("Select .bdt file or Cancel to skip."), "./", ".bdt Files (*.bdt)");
         if (fName.length())
         {
            fName = QFileInfo(fName).fileName();
            charbytes = fName.toLatin1();
            ui->threeDJmpTerm->defaultResponse(charbytes);
         }
      }
      else
      {
         ui->threeDJmpTerm->fakeEnter();
      }
   }
   ui->threeDJmpTerm->activateWindow();
   ui->threeDJmpTerm->setFocus();
}


void GravityGui::prog3DJmpDone(int code,QProcess::ExitStatus exit_status)
{
   if (ui)
   {
      ui->termTab->tabBar()->setTabTextColor(TABS::THREEDJMP,tabBlack);
      ui->threeDJmpButton->setEnabled(true);
   }
}

// Direct 3D family of functions. Only one of these can run at a time.
// The button event handler passes the program name.
void GravityGui::doDirect3d(QString progname)
{
      // just one instance
   if (prog3d && prog3d->progIsRunning() != QProcess::NotRunning)
      return;
   prog3d = make_unique<GravityProg>(this,ui->direct3dTerm,progname);
   connect(prog3d.get(), static_cast<void(GravityProg::*)(int,QProcess::ExitStatus)>(&GravityProg::progDone), this, [=](int code,QProcess::ExitStatus exit_status){prog3dDone(code,exit_status);});
   connect(prog3d.get(), static_cast<void(GravityProg::*)(QByteArray)>(&GravityProg::progStdOutText), this, [=](QByteArray text){prog3dGotLine(text);});

   direct3dSwitch();

   if (prog3d->progInvoke())
   {
      ui->termTab->tabBar()->setTabTextColor(TABS::DIRECT3D,tabRunning);
      // disable all of the buttons.
      ui->direct3d_bl->setEnabled(false);
      ui->direct3d_bl_mp->setEnabled(false);
      ui->direct3d_bl_sig_01->setEnabled(false);
      ui->direct3d_bl_sig_05->setEnabled(false);
      ui->direct3d_bl_sub->setEnabled(false);
   }
}

void GravityGui::prog3dGotLine(QByteArray stuff)
{
   QString fName;
   QByteArray charbytes;

   ui->direct3dTerm->activateWindow();
   ui->direct3dTerm->setFocus();
   if (stuff.contains("INPUT *.dir filename"))
   {
      if (ui->baseName->text().length())
      {
         if (ui->filePrompt->isChecked())
           fName = QFileDialog::getOpenFileName(this,
                         tr("Select .dir file."), "./", ".dir Files (*.dir)");
         if (!fName.length())
            fName = ui->baseName->text() + ui->fnameMod->text() + ".dir";
         else
            fName = QFileInfo(fName).fileName();
         charbytes = fName.toLatin1();
         ui->direct3dTerm->defaultResponse(charbytes);
      }
      else
      {
         ui->direct3dTerm->printWarn("There is no base filename.\nLoad a parameter file, a .gdt file,\nor enter a base filename manually.");
         prog3d->terminateProg();
         return;
      }
   }
}


void GravityGui::prog3dDone(int code,QProcess::ExitStatus exit_status)
{
   if (ui)
   {
      ui->termTab->tabBar()->setTabTextColor(TABS::DIRECT3D,tabBlack);
      ui->direct3d_bl->setEnabled(true);
      ui->direct3d_bl_mp->setEnabled(true);
      ui->direct3d_bl_sig_01->setEnabled(true);
      ui->direct3d_bl_sig_05->setEnabled(true);
      ui->direct3d_bl_sub->setEnabled(true);
   }
}
