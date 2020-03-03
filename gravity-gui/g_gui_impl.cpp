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




/* The implementation part of gravity_gui. 
   The designer adds stuff to other files, we pass calls from the control
   handlers to the real handlers in here.
*/


#include "gravity_gui.h"
#include "ui_gravity_gui.h"
#include <string>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <typeinfo>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <QFileDialog>
#include <QRegularExpression>
#include <QMessageBox>
#include <QFont>
#include <QFontInfo>
#include <QFontDialog>
#include <QFontMetrics>
#include <QSettings>
#include <QSize>
#include <QWindow>
#include <QPixmap>
#include <QScreen>
#include <QDate>
#include <QTime>
#include <QDesktopWidget>
#include <curses.h>
#include <term.h>
#include "g_prog.h"
#include "helpbox.h"

using namespace std;

void GravityGui::setupImpl()
{
   neuroMapper = new QSignalMapper(this);
   connect(neuroMapper,SIGNAL(mapped(int)),this,SLOT(neuroClicked(int)));
   analogMapper = new QSignalMapper(this);
   connect(analogMapper,SIGNAL(mapped(int)),this,SLOT(analogClicked(int)));

   loadSettings();
   initParams();
   paramsClean();
   rebuildRecents();
}

void GravityGui::loadSettings()
{
   QSettings settings("gravity","gravity_settings");
   if (!settings.status() && settings.contains("geometry")) // does it exist at all?
   {
      QFont font;
      QString sessionDir;

      restoreGeometry(settings.value("geometry").toByteArray());
      restoreState(settings.value("windowState").toByteArray());
      font.fromString(settings.value("buttonfont").toString());
      setRunButtonFont(font);
      font.fromString(settings.value("labelfont").toString());
      setLabelFont(font);
      font.fromString(settings.value("otherbuttonfont").toString());
      setOtherButtonFont(font);
      font.fromString(settings.value("inputfont").toString());
      setInputFont(font);
      ui->surrSeed->setText(settings.value("seedvalue").toString());
      recentProjs = settings.value("recentProjs").toStringList();
      sessionDir = settings.value("currentsession").toString();
      for (int entry = 0; entry < MAX_RECENTS; ++entry) // recent projects in file menu
      {
         menuProjs[entry] = new QAction(this);
         menuProjs[entry]->setVisible(false);
         connect(menuProjs[entry],SIGNAL(triggered()),this,SLOT(OpenRecentProj()));
         ui->menuFile->addAction(menuProjs[entry]);
      }
      if (QDir::setCurrent(sessionDir))
      {
         ui->currentSession->setText(sessionDir);
         updateRecents();
      }
   }
}


void GravityGui::saveSettings()
{
   QSettings settings("gravity","gravity_settings");
   if (!settings.status())
   {
      settings.setValue("geometry",saveGeometry());
      settings.setValue("windowState",saveState());
      settings.setValue("buttonfont",ui->gBatch->font().toString());
      settings.setValue("labelfont",ui->baseLabel->font().toString());
      settings.setValue("otherbuttonfont",ui->paramLoadFile->font().toString());
      settings.setValue("inputfont",ui->currentSession->font().toString());
      settings.setValue("seedvalue",ui->surrSeed->text());
      settings.setValue("currentsession",ui->currentSession->text());
      settings.setValue("recentProjs",recentProjs);
   }
}

void GravityGui::initParams()
{
      // combo box
   ui->shiftValues->clear();
   ui->shiftValues->setEditable(true);
   ui->shiftValues->lineEdit()->setReadOnly(true);
   ui->shiftValues->lineEdit()->setAlignment(Qt::AlignRight);
   ui->shiftValues->addItem("100","100");
   ui->shiftValues->addItem("1000","1000");
   ui->shiftValues->addItem("10000","10000");
   for (int i = 0 ; i < ui->shiftValues->count() ; ++i) 
      ui->shiftValues->setItemData(i, Qt::AlignRight, Qt::TextAlignmentRole);
   ui->shiftValues->setCurrentIndex(0);

   ui->acceptorValues->clear();
   ui->acceptorValues->addItem("Forward (+1)","1");
   ui->acceptorValues->addItem("Backward (-1)","-1");
   ui->acceptorValues->setCurrentIndex(0);

   ui->effectorValues->clear();
   ui->effectorValues->addItem("Forward (+1)","1");
   ui->effectorValues->addItem("Backward (-1)","-1");
   ui->effectorValues->setCurrentIndex(0);

   ui->forceSign->clear();
   ui->forceSign->addItem("Excitation (+1.0)","1.0");
   ui->forceSign->addItem("Inhibition (-1.0)","-1.0");
   ui->forceSign->setCurrentIndex(0);

   ui->gravityOpts->clear();
   ui->gravityOpts->setEditable(true);
   ui->gravityOpts->lineEdit()->setReadOnly(true);
   ui->gravityOpts->lineEdit()->setAlignment(Qt::AlignCenter);
   ui->gravityOpts->addItem("N","N");
   ui->gravityOpts->addItem("O","O");
   ui->gravityOpts->addItem("P","P");
   ui->gravityOpts->addItem("Q","Q");
   for (int i = 0 ; i < ui->gravityOpts->count() ; ++i) 
      ui->gravityOpts->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
   ui->gravityOpts->setCurrentIndex(0);
}

void GravityGui::OpenRecentProj()
{
   QString proj;
   QAction *action = dynamic_cast<QAction *>(sender());  
   if (action)
   {
      checkDirty();
      proj = action->data().toString();
      QDir session(proj);
      if (session.exists())
      {
         ui->currentSession->setText(session.absolutePath());
         QDir::setCurrent(session.absolutePath());
      }
      else
      {
         QMessageBox msgBox(this);
         msgBox.setText("This project directory does not exist.\nDo you want to delete it from the session list?"); 
         msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
         msgBox.setIcon(QMessageBox::Question);
         int result = msgBox.exec();
         if (result == QMessageBox::Yes)
         {
            removeRecent(proj);
            ui->currentSession->clear(); 
         }
      }
   }
}

void GravityGui::actionQuit()
{
   checkDirty();
   saveSettings();
   close();
}

void GravityGui::doHelp()
{
   helpbox *help = new helpbox(this);
   help->exec();
}

void GravityGui::closeEvent(QCloseEvent *evt)
{
   actionQuit();
   evt->accept();
}

void GravityGui::doClearRecents()
{
   recentProjs.clear();
   rebuildRecents();

}
// we put all window captures into this dir
void GravityGui::createCapture()
{
   QDir cwd;
   QString wd = cwd.absolutePath();
   QString caps = wd + "/" + capDir; 
   cwd.setPath(caps);
   if (!cwd.exists())
      cwd.mkdir(caps);
}

// I tried several ways to change the fonts for a type of control, using
// Qt hints, but was not able to get it to work reliably, not sure why.
// So, do what works which is to change every control. As new ones are
// added, have to remember to add them to one of the change font functions.
void GravityGui::doRunButtonFont()
{
   bool ok;
   QFont font = ui->gBatch->font();
   QFont newFont = QFontDialog::getFont(&ok,font,this);
   if (ok)
      setRunButtonFont(newFont);
}

void GravityGui::setRunButtonFont(const QFont& font)
{
   ui->gBatch->setFont(font);
   ui->xtrydisButton->setFont(font);
   ui->xprojtmButton->setFont(font);
   ui->surrogatesButton->setFont(font);
   ui->gsigButton->setFont(font);
   ui->direct3d_bl->setFont(font);
   ui->xslopeButton->setFont(font);
   ui->direct3d_bl_mp->setFont(font);
   ui->direct3d_bl_sub->setFont(font);
   ui->direct3d_bl_sig_05->setFont(font);
   ui->direct3d_bl_sig_01->setFont(font);
   ui->fireworksButton->setFont(font);
   ui->threeDJmpButton->setFont(font);
   ui->spkpat6bg->setFont(font);
   ui->spkpat6bgr->setFont(font);
   ui->spkpat6kbg->setFont(font);
   ui->spkpatdist->setFont(font);
   ui->spkpatwip->setFont(font);
}

void GravityGui::doLabelFont()
{
   bool ok;
   QFont font = ui->baseLabel->font();
   QFontDialog selFont(font,this);
   selFont.setCurrentFont(font);
   QFont newFont = selFont.getFont(&ok,font,this);
   if (ok)
      setLabelFont(newFont);
}

void GravityGui::setLabelFont(const QFont& font)
{
   ui->baseLabel->setFont(font);
   ui->fnameModLabel->setFont(font);
   ui->sessionLabel->setFont(font);
   ui->seedLabel->setFont(font);
   ui->baseLabel->setFont(font);
   ui->fnameModLabel->setFont(font);
   ui->numParticlesLabel->setFont(font);
   ui->timeStepLabel->setFont(font);
   ui->normFactorLabel->setFont(font);
   ui->slideLabel->setFont(font);
   ui->forceSignLabel->setFont(font);
   ui->acceptorLabel->setFont(font);
   ui->rateNormLabel->setFont(font);
   ui->effectorLabel->setFont(font);
   ui->forwardTauLabel->setFont(font);
   ui->forwardChgIncLabel->setFont(font);
   ui->backChargeLabel->setFont(font);
   ui->wellDiamLabel->setFont(font);
   ui->gravityOptsLabel->setFont(font);
   ui->shiftLabel->setFont(font);
   ui->neuroLabel->setFont(font);
   ui->analogLabel->setFont(font);
   ui->backwardTauLabel->setFont(font);
   ui->timeSpanLabel->setFont(font);
   ui->filePrompt->setFont(font);
   QFont tabFont(font);
   tabFont.setBold(true);              // the terminal tab labels look much better bold
   ui->termTab->setFont(tabFont);
}

void GravityGui::doOtherButtonFont()
{
   bool ok;
   QFont font = ui->paramLoadFile->font();
   QFont newFont = QFontDialog::getFont(&ok,font,this);
   if (ok)
      setOtherButtonFont(newFont);
}


void GravityGui::setOtherButtonFont(const QFont& font)
{
   ui->sessionButton->setFont(font);
   ui->gdtFile->setFont(font);
   ui->paramSaveFile->setFont(font);
   ui->paramLoadFile->setFont(font);
   ui->buttonQuit->setFont(font);
   ui->quitGProg->setFont(font);
   ui->winCap->setFont(font);
   ui->openViewer->setFont(font);
}

void GravityGui::doInputFont()
{
   bool ok;
   QFont font = ui->currentSession->font();
   QFont newFont = QFontDialog::getFont(&ok,font,this);
   if (ok)
      setInputFont(newFont);
}

void GravityGui::setInputFont(const QFont& font)
{
   ui->currentSession->setFont(font);
   ui->gdtFullName->setFont(font);
   ui->paramFullName->setFont(font);
   ui->surrSeed->setFont(font);
   ui->shiftValues->setFont(font);
   ui->baseName->setFont(font);
   ui->fnameMod->setFont(font);
   ui->timeStep->setFont(font);
   ui->slideValue->setFont(font);
   ui->normFactor->setFont(font);
   ui->acceptorValues->setFont(font);
   ui->effectorValues->setFont(font);
   ui->forceSign->setFont(font);
   ui->rateNorm->setFont(font);
   ui->forwardTau->setFont(font);
   ui->forwardInc->setFont(font);
   ui->backCharge->setFont(font);
   ui->wellDiam->setFont(font);
   ui->gravityOpts->setFont(font);
   ui->selParticles->setFont(font);
   ui->backwardTau->setFont(font);
   ui->timeSpan->setFont(font);

   ui->neuroChans->setFont(font);
   QFontInfo finfo(font);
   int box;
   if (finfo.pixelSize() < 11)
      box = finfo.pixelSize();
   else
      box = finfo.pixelSize()+2;
   QString style;
   QTextStream(&style) << "QCheckBox::indicator { width: " << box << "px; height: " << box << "px;}";
   for (int row = 0 ; row < ui->neuroChans->count(); ++row)
   {
      QListWidgetItem *item = ui->neuroChans->item(row);
      QObject *obj = ui->neuroChans->itemWidget(item);
      QCheckBox *cb = dynamic_cast<QCheckBox*>(obj);
      if (cb)
      {
         cb->setFont(font);
         cb->setStyleSheet(style);
      }
   }
   ui->analogChans->setFont(font);
   for (int row = 0 ; row < ui->analogChans->count(); ++row)
   {
      QListWidgetItem *item = ui->analogChans->item(row);
      QObject *obj = ui->analogChans->itemWidget(item);
      QCheckBox *cb = dynamic_cast<QCheckBox*>(obj);
      if (cb)
      {
         cb->setFont(font);
         cb->setStyleSheet(style);
      }
   }
}

// pick/create a working directory
void GravityGui::doSession()
{
   QDir session;
   QFileDialog dirs(this,tr("Select Existing Working Directory or Create A New One"));
   dirs.setFileMode(QFileDialog::Directory);
   dirs.setOption(QFileDialog::ShowDirsOnly,true);
   dirs.setDirectory(session.absolutePath());
   if (dirs.exec())
   {
      session = dirs.directory();
      ui->currentSession->setText(session.absolutePath());
      QDir::setCurrent(session.absolutePath()); // make this the cwd
      updateRecents();
   }
}


// Create a .gdt file from a .adt, .bdt or .edt file
void GravityGui::makeGDT()
{
   QString msg;
   QString line;
   int chan,time;
   chanList bdtList;
   chanListPair cl_pair;
   int t_wid = 8;      // adt is I2 I8, bdt is I5 I8, edt is I5 I10
   int chan_len = 5;   // we turn edt into bdt, so t_wid is same for both adt & bdt
   QString all;

   auto addChan = [&bdtList](int chan) {
      chanListPair cl_pair; if (chan < 4096) { 
         cl_pair = bdtList.emplace(chan,0); 
         ++(cl_pair.first->second);
      } 
   };

   gbatchSwitch();  // bring our terminal to foreground.
   if (ui->currentSession->text().length() == 0)
   {
      QMessageBox msgBox(this);
      msgBox.setText("You need to start a session before you can create a .gdt file."); 
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.setIcon(QMessageBox::Information);
      msgBox.exec();
      return;
   }

   QString fName = QFileDialog::getOpenFileName(this,
                      tr("Select .adt .bdt or edt file."), "./", ".adt .bdt .edt Files (*.adt *.bdt *.edt)");
   if (!fName.length())
      return;

   QFileInfo readInfo(fName);
   QString justName =readInfo.completeBaseName();
   QString outname = justName + ".gdt";
   QString suff = readInfo.suffix();
   FTYPE ftype;
   if (suff == "adt")
   {
      ftype = FTYPE::ADT;
      chan_len = 2;
   }
   else if (suff == "bdt")
   {
      ftype = FTYPE::BDT;
   }
   else if (suff == "edt")
   {
      QMessageBox msgBox(this);
      msgBox.setText("You have selected a .edt file.\nGravity can only process .adt and .bdt files."); 
      msgBox.setInformativeText("Do you want to create a new .bdt file in the current session directory file from the .edt file?");
      msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
      msgBox.setIcon(QMessageBox::Question);
      int resp = msgBox.exec();
      if (resp == QMessageBox::No)
      {
         ui->gbatchTerm->printWarn("Creating a .gdt file cancelled.\n");
         return;
      }
      QString bdtName = QDir::currentPath() + "/" + justName + ".bdt";
      if (!edt2bdt(fName,bdtName))
         return;
      ftype = FTYPE::BDT;
      fName = bdtName;
   }
   else
   {
      ui->gbatchTerm->printWarn("This is not a recognized file type. Try again.");
      return;
   }

   while (true)
   {
      outname = QFileDialog::getSaveFileName(this,
                      tr("Save GDT To. . ."), outname, tr("*.gdt"));
      if (!outname.length())
         return;
      QFileInfo outInfo(outname);
      if (outInfo.completeBaseName().length() > GBATCH_MAX_FNAME)
         warnTooLong(outInfo.completeBaseName());
      else
         break;
   }

   QFile file(fName);
   if (!file.open(QIODevice::ReadOnly))
   {
      QTextStream(&msg) << tr("Error opening file ") << fName << endl << tr("Error is:               ") << file.errorString() << endl;
      ui->gbatchTerm->printWarn(msg);
      return;
   }
   QFile outfile(outname);
   if (!outfile.open(QIODevice::WriteOnly))
   {
      QTextStream(&msg) << tr("Error opening file ") << outname << endl << tr("Error is:               ") << file.errorString() << endl;
      ui->gbatchTerm->printWarn(msg);
      return;
   }
    // Finally! read file
   QTextStream stream(&outfile);
   QTextStream(&msg) << tr("Loading ") << fName << endl;
   ui->gbatchTerm->append(msg);
   ui->gbatchTerm->repaint();
   msg.clear();
   try
   {
      all = file.readAll();
   }
   catch (std::bad_alloc) // QStrings can only hold so much
   {
      QTextStream(&msg) << tr("This file is too large.") << endl << tr("Use scope or another program to create a file with a subset of this file.") << endl;
      ui->gbatchTerm->printWarn(msg);
      return;
   }
   QStringList rows = all.split('\n',QString::SkipEmptyParts);
   QStringList::const_iterator iter = rows.begin();
   if (ftype == FTYPE::BDT && *(iter) != "   11 1111111" && *(iter+1) != "   11 1111111")
   {
      QTextStream(&msg) << tr("This not a valid .bdt file ") << fName << endl;
      ui->gbatchTerm->printWarn(msg);
      msg.clear();
      file.close();
      return;
   }

   if (ftype == FTYPE::BDT) // pick up 1st time
   {
      iter += 2;     // skip header
      line = *iter;
   }
   else // adt
      line = *iter;

   chan = line.midRef(0,chan_len).toInt();
   time = line.mid(chan_len,-1).toInt();
   if (time - 1 > 0)                // insert gdt start mark at earliest time
      --time;
   else
      time = 0;
   QTextStream(&msg) << tr("Saving ") << outname << endl;
   ui->gbatchTerm->append(msg);
   QApplication::setOverrideCursor(Qt::WaitCursor);
   ui->gbatchTerm->repaint();
   msg.clear();
   if (ftype == FTYPE::BDT)
   {
      stream << "   11 1111111" << endl; // write header
      stream << "   11 1111111" << endl;
   }
   stream.setFieldAlignment(QTextStream::AlignRight);
   stream << qSetFieldWidth(chan_len) << GDT_START << qSetFieldWidth(t_wid) << time << qSetFieldWidth(0) << endl;
   stream.setFieldWidth(0);
   stream << line << endl;
   ++iter;

   addChan(chan);

   for (; iter != rows.end(); ++iter)
   {
      line = *iter;
      stream << line << endl;
      chan = line.midRef(0,chan_len).toInt();
      time = line.mid(chan_len,-1).toInt();
      addChan(chan);
   }
   stream << qSetFieldWidth(chan_len) << GDT_END << qSetFieldWidth(t_wid) << ++time << qSetFieldWidth(0) <<endl;

   setBaseMod(justName);

   bool maxSpikes = false;
   for (auto &chan : bdtList)
   {
      if (chan.second > MAX_SPIKES)
      {
         maxSpikes = true;
         msg.clear();
         QTextStream(&msg) << tr("Channel ") << chan.first << tr(" has ") << chan.second << tr(" spikes.") << endl;
         ui->gbatchTerm->append(msg);
      }
   }
   if (maxSpikes)
   {
      QTextStream(&msg) << tr("Warning: Some of the gravity programs limit the maximum number of spikes to ") << MAX_SPIKES << "." << endl << tr("Some programs may hang or crash if you use this .gdt file") << endl << tr("Use scope or other programs to create a shorter .edt, .bdt or .adt file and make a new .gdt file.") << endl;
      ui->gbatchTerm->printWarn(msg);
      msg.clear();
   }

     // to pick up analog channels, create a symbolic link to adt, .bdt file. 
     // If we created a local .bdt from a .edt file, the link call will
     // fail, but we don't care.
   QFile::link(fName,readInfo.fileName());
   QTextStream(&msg) << tr("Done.") << endl;
   ui->gbatchTerm->append(msg);
   QApplication::restoreOverrideCursor();
}

// Load gdt file button clicked. If we are loading this directly, it may be a
// .gdt file that we are going to use the existing parameters for. The
// currently selected chans are likely to be not the same for the new .gdt
// file, and may not even exist. So clear out any selected chans.
void GravityGui::gdtFileOpen()
{
   QString msg;

   gbatchSwitch();
   checkDirty();

   QString fName = QFileDialog::getOpenFileName(this,
                      tr("Select .gdt file."), "./", ".gdt Files (*.gdt)");
   QFileInfo readInfo(fName);
   if (readInfo.completeBaseName().length() > GBATCH_MAX_FNAME)
   {
      warnTooLong(readInfo.completeBaseName());
      return;
   }
   gdtFileLoad(fName);
   selectedChans.clear();
   validateGDT();
   for (int row = 0 ; row < ui->neuroChans->count(); ++row)
   {
      QListWidgetItem *item = ui->neuroChans->item(row);
      QObject *obj = ui->neuroChans->itemWidget(item);
      QCheckBox *cb = dynamic_cast<QCheckBox*>(obj);
      if (cb)
         cb->setCheckState(Qt::Unchecked);
   }
   paramsDirty();
}

// Two ways to get here, via button click or from loading a param file.
// The .gdt file, if it exists, sets the basename for other filenames we
// create or pass on to other programs.
void GravityGui::gdtFileLoad(QString fName)
{
   QString msg;

   if (fName.length())
   {
      QFileInfo readInfo(fName);
      QString justName =readInfo.completeBaseName();
      gdtSelFName = readInfo.fileName();
      ui->gdtFullName->setText(gdtSelFName);
      setBaseMod(justName);
      QFile file(fName);
      if (!file.open(QIODevice::ReadOnly))
      {
         QTextStream(&msg) << tr("Error opening file ") << fName << endl << tr("Error is:               ") << file.errorString() << endl;
         ui->gbatchTerm->printWarn(msg);
      }
      else
      {
         QApplication::setOverrideCursor(Qt::WaitCursor);
         ui->gbatchTerm->repaint();
         QTextStream(&msg) << tr("Loading ") << fName << endl;
         ui->gbatchTerm->append(msg);
         QDir::setCurrent(readInfo.canonicalPath()); // make src the cwd
         QString all = file.readAll();
         if (!makeChanList(all))
         {
            QString msg;
            QTextStream(&msg) << tr("This not a valid .gdt file ") << fName << endl;
            ui->gbatchTerm->printWarn(msg);
            file.close();
            return;
         }
         file.close();
         haveGDT=true;
         ui->gBatch->setEnabled(true);
         QApplication::restoreOverrideCursor();
      }
   }
}

// Load param button click
void GravityGui::paramLoad()
{
   int int_val;
   int num_particles;
   QString msg;

   gbatchSwitch();
   checkDirty();

   QString fName = QFileDialog::getOpenFileName(this,
                      tr("Select Parameter File To Load."), "./", tr("Parameter Files (param* *.prm)"));
   if (fName.length())
   {
      paramFName = fName;
      QFileInfo readInfo(fName);
      QString justName =readInfo.fileName();
      QFile file(fName);
      if (!file.open(QIODevice::ReadOnly))
      {
         QTextStream(&msg) << tr("Error opening file ") << fName << endl << tr("Error is:               ") << file.errorString();
         ui->gbatchTerm->printWarn(msg);
      }
      else
      {
         selectedChans.clear();
         QDir::setCurrent(readInfo.canonicalPath()); // make src the cwd
         ui->paramFullName->setText(justName);
         QTextStream(&msg) << tr("Loading ") << justName << endl;
         ui->gbatchTerm->append(msg);
         QString all = file.readAll();
           // ignore blank lines or lines with just ws
         QStringList rows = all.split(QRegularExpression("\\s+"),QString::SkipEmptyParts);
         QStringList::const_iterator iter = rows.begin();
         QString t1=*iter;
         QString t2=*(iter+1);
         if (rows.size() < PARAM_LINES || !(*iter).contains("100") || !(*(iter+1)).contains(".gout"))
         {
            QString msg;
            QTextStream(&msg) << tr("FATAL: This does not seem to be a valid parameter file.") << endl;
            ui->gbatchTerm->printWarn(msg);
            file.close();
            return;
         }

          // pick up info from each line of text
         int_val = ui->shiftValues->findText(*(iter+SHIFT));
         ui->shiftValues->setCurrentIndex(int_val);
         num_particles = (*(iter+PARTICLES)).toInt();
         ui->timeStep->setValue((*(iter+TIMESTEP)).toDouble());
         ui->slideValue->setText(*(iter+SLIDE));
         ui->normFactor->setValue((*(iter+NORM)).toInt());
         int_val = ui->acceptorValues->findData((*(iter+ACCEPTOR)).toInt());
         ui->acceptorValues->setCurrentIndex(int_val);
         int_val = ui->effectorValues->findData((*(iter+EFFECTOR)).toInt());
         ui->effectorValues->setCurrentIndex(int_val);
         int_val = ui->forceSign->findData((*(iter+FORCE)).toDouble());
         ui->forceSign->setCurrentIndex(int_val);
         ui->forwardTau->setValue((*(iter+FWD_TAU)).toDouble());
         ui->backwardTau->setValue((*(iter+BCK_TAU)).toDouble());
         ui->forwardInc->setValue((*(iter+FWD_CHG)).toDouble());
         ui->backCharge->setValue((*(iter+BCK_CHG)).toDouble());
         ui->wellDiam->setValue((*(iter+WELL_DIAM)).toDouble());
         int_val = ui->gravityOpts->findText(*(iter+OPTIONS));
         ui->gravityOpts->setCurrentIndex(int_val);
         for (int chans = 0; chans < num_particles; ++chans)
            selectedChans.insert((*(iter+P1_END+chans)).toInt());
         ui->timeSpan->setValue((*(iter+P1_END+num_particles+TIMESPAN)).toDouble());
         gdtParamFName = *(iter + P1_END+num_particles+INFILE);
         gdtFileLoad(gdtParamFName);
          // the user can just load a .gdt file, in which case, basename and mod
          // come from that filename. If using a param file, use it as the source
          // of these items. Over-write what gdtFileLoad did.
         setBaseMod(readInfo.completeBaseName());
         checkSelected();
         file.close();
         paramsClean();
      }
   }
}

// We loaded a param file. If it has chans and they are in our checkbox
// list, check them.
void GravityGui::checkSelected()
{
   if (selectedChans.size() == 0)
      return;
   if (ui->neuroChans->count() == 0)
      return;

   validateGDT();
     // check the ones that are in the gdt file and also the param file
   for (int row = 0 ; row < ui->neuroChans->count(); ++row)
   {
      QListWidgetItem *item = ui->neuroChans->item(row);
      QObject *obj = ui->neuroChans->itemWidget(item);
      QCheckBox *cb = dynamic_cast<QCheckBox*>(obj);
      if (cb)
      {
         QVariant curr = cb->property("channum");
         for (auto &val: selectedChans)
         {
            if (val == curr)
            {
               cb->setCheckState(Qt::Checked);
               break;
            }
         }
      }
   }
}


// Sanity checking for .gdt file
// 1. Check to see if  there are chans in the selected
//    list (if we have one) that are not in the .gdt file. 
// 2. There is a limit in downstream programs of a max of 10000 spikes 
//    on a channel. If so, warn user.
// These are not fatal errors, but warrant warnings.
void GravityGui::validateGDT()
{
   QString msg;
   bool have_val;
   int sel_chans = 0;

   // validate the list. If there are chans in the selected
   // list that are not in the .gdt file, warn the user and remove from selected list
   for (selChanListIter iter = selectedChans.begin(); iter != selectedChans.end() ; ) 
   {
      have_val = false;
      for (int row = 0 ; row < ui->neuroChans->count(); ++row)
      {
         QListWidgetItem *item = ui->neuroChans->item(row);
         QObject *obj = ui->neuroChans->itemWidget(item);
         QCheckBox *cb = dynamic_cast<QCheckBox*>(obj);
         if (cb)
         {
            QVariant curr = cb->property("channum");
            if (*iter == curr)
            {
               have_val = true;
               ++sel_chans;
               break;
            }
         }
      }
      if (!have_val)
      {
         msg.clear();
         QTextStream(&msg) << tr("Warning: Channel ") << *iter << tr(" is not in the .gdt file.") << endl;
         ui->gbatchTerm->printWarn(msg);
         iter = selectedChans.erase(iter);
      }
      else
         ++iter;
   }
   ui->selParticles->setText(QString::number(selectedChans.size()));
   bool maxSpikes = false;
   for (auto &chan : currChans)
   {
      if (chan.second > MAX_SPIKES)
      {
         maxSpikes = true;
         msg.clear();
         QTextStream(&msg) << ("Channel ") << chan.first << tr(" has ") << chan.second << tr(" spikes.") << endl;
         ui->gbatchTerm->printWarn(msg);
      }
   } 
   if (maxSpikes)
   {
      QTextStream(&msg) << tr("Warning: Some of the gravity programs limit the maximum number of spikes to ") << MAX_SPIKES << "." << endl << tr("Some programs may hang or crash if you use this .gdt file") << endl << tr("Use scope or other programs to create a shorter .bdt or .adt file and make a new .gdt file.") << endl;
      ui->gbatchTerm->printWarn(msg);
   }
}


// save current active program's window (if there is one) bitmap to a file
// We create save dir here instead of other places because we can easily
// polute lots of dirs with the save dir. If they want to capture, they must
// intend to save.
void GravityGui::doWinCap()
{
   int winId=0;
   QString title;
   QString saveName, nameToSave;
   QString msg;
   int currTerm;

   auto bailout=[this](int &currTerm){ui->termTab->setCurrentIndex(currTerm); return;};

   createCapture();  // may need to do this
   currTerm = ui->termTab->currentIndex();
   saveSwitch();
   ui->saveTerm->append("\nSave a window's contents to a file.\n");
   ui->saveTerm->append("Select a window to capture and save to a file\nby clicking on the window.\n");
   ui->saveTerm->printWarn("\nWaiting for mouse click. . .\n");
   QApplication::processEvents();

   QProcess getid;
   QString cmd("xwininfo");
   QStringList params({"-int","-children"}); //if you want the frame included,
   getid.start(cmd,params);                  // use Parent window id
   getid.waitForFinished();
   QString output(getid.readAllStandardOutput());
   getid.close();
   QStringList rows = output.split('\n',QString::SkipEmptyParts);
   int idline = rows.indexOf(QRegExp(".*Window id.*"));
   if (idline >= 0) // sample output: xwininfo: Window id: 0x243 "xtrydis"
   {
      QStringList idrows = rows[idline].split(' ',QString::SkipEmptyParts);
      winId = idrows[3].toInt();
      title = idrows[4];
      title.replace("\"","");
      // since the user can click on anything, toolbar, desktop, the title could be 
      // an invalid file name, so only use the ones we know.
      int known = knownProgs.indexOf(title);
      if (known >= 0)
         title = knownProgs[known];
      else
         title = "UnknownWin";
   }
   else
   {
      ui->saveTerm->printWarn("That was not a window, aborting.");
      bailout(currTerm);
   }
   QTextStream(&msg) << tr("Window ") << title << tr(" selected.") << endl;
   ui->saveTerm->append(msg);
   msg.clear();
   nameToSave = capDir + "/" +
                title + "_" + 
                ui->baseName->text() + ui->fnameMod->text() + "_" +
                QDate::currentDate().toString("yyyy-MM-dd") + "_" + 
                QTime::currentTime().toString("hh:mm:ss") + 
                ".jpg";
   saveName = QFileDialog::getSaveFileName(this,
                      tr("Save Window To File"), nameToSave, tr("jpg* *.jpg"));
   if (!saveName.length())
   {
      ui->saveTerm->printWarn("Saving cancelled\n");
      bailout(currTerm);
   }
   QTextStream(&msg) << tr("Saving to ") << saveName << endl;
   ui->saveTerm->append(msg);
   msg.clear();

   QScreen *screen = QGuiApplication::primaryScreen();
   if (const QWindow *window = windowHandle())
       screen = window->screen();
   if (!screen) // this means there are no monitor(s)
      bailout(currTerm);
   QPixmap cap = screen->grabWindow(winId);
   if (!cap.isNull())
   {
      cap.save(saveName,0,100);
      ui->saveTerm->append("Saved.");
   }
   bailout(currTerm);
}

// launch the default viewer in the capture dir
// if there is one, else in whatever cwd is.
void GravityGui::doOpenViewer()
{
   QString msg;
   QDir test("./" + capDir);
   if (!test.exists())
   {
      saveSwitch();
      QTextStream(&msg) << tr("The save directory \"") << capDir << tr("\" does not exist in the current project directory.") << endl;
      QTextStream(&msg) << tr("Save some screen captures first.") << endl;
      ui->saveTerm->printWarn(msg);
      msg.clear();
      return;
   }
   QString cmd = "xdg-open " + capDir;
   int res = system(cmd.toLatin1().data());
   if (res != 0)
   {
      saveSwitch();
      QTextStream(&msg) << tr("The default file viewer returned error code ") << res<< endl;
      ui->saveTerm->printWarn(msg);
   }
}

void GravityGui::paramSave()
{
   QString msg;
   QMessageBox msgBox(this);

   gbatchSwitch();
   if (gdtSelFName.length() == 0)
   {
      msgBox.setText("The .gdt file is missing.");
      msgBox.setInformativeText("You must select a .gdt file or load a parameter file before you can save the parameters. File not saved.");
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.setIcon(QMessageBox::Information);
      msgBox.exec();
      return;    
   }
   if (selectedChans.size() < 2)
   {
      msgBox.setText("You have to select at least two neuron channels before you can save a param file. File not saved.");
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.setIcon(QMessageBox::Information);
      msgBox.exec();
      return;
   }

   QString nameToSave;
   if (paramFName.length())
      nameToSave = paramFName;
   else
      nameToSave = "./";

   QString fName = QFileDialog::getSaveFileName(this,
                      tr("Save Parameters"), nameToSave, tr("param* *.prm"));
   QFileInfo readInfo(fName);
   if (readInfo.completeBaseName().length() > GBATCH_MAX_FNAME)
   {
      warnTooLong(readInfo.completeBaseName());
      return;
   }

   if (fName.length())
   {
      QString params = buildParams();
      QFileInfo readInfo(fName);
      QString justName =readInfo.fileName();
      ui->paramFullName->setText(justName);
      QFile file(fName);
      if (!file.open(QIODevice::WriteOnly))
      {
         QTextStream(&msg) << tr("Error opening file ") << fName << endl << tr("Error is:               ") << file.errorString();
         ui->gbatchTerm->printWarn(msg);
      }
      else
      {
         paramFName = fName;
         QTextStream stream(&file);
         stream << params;
         file.close();
         paramsClean();
      }
      ui->gbatchTerm->append("Done.\n");
   }
}

// Collect the contents of various vars and controls and return text
// that has to same format as a param file.
QString GravityGui::buildParams()
{
   int int_val;
   QString text;
   QString params;
   QTextStream stream(&params);
   QString basename = ui->baseName->text();
   QString modstr = ui->fnameMod->text();

   int_val = ui->shiftValues->currentIndex();
   text = ui->shiftValues->itemText(int_val);
   stream << text << endl;
   stream << basename + modstr + ".gout" << endl;
   stream << selectedChans.size() << endl;
   stream << ui->timeStep->textFromValue(ui->timeStep->value()) << endl;
   stream << ui->slideValue->text() << endl;
   stream << ui->normFactor->text() << endl;
   stream << (ui->acceptorValues->currentData()).toString() << endl;
   stream << (ui->effectorValues->currentData()).toString() << endl;
   stream << (ui->forceSign->currentData()).toString() << endl;
   stream << "3" << endl;   // rate norm is constant
   stream << ui->forwardTau->textFromValue(ui->forwardTau->value()) << endl;
   stream << ui->backwardTau->textFromValue(ui->backwardTau->value()) << endl;
   stream << ui->forwardInc->textFromValue(ui->forwardInc->value()) << endl;
   stream << ui->backCharge->textFromValue(ui->backCharge->value()) << endl;
   stream << ui->wellDiam->textFromValue(ui->wellDiam->value()) << endl;
   int_val = ui->gravityOpts->currentIndex();
   text = ui->gravityOpts->itemText(int_val);
   stream << text << endl;
   for (auto &chan : selectedChans)
      stream << chan << endl;
   stream << "y" << endl;     // "yes" to a prompt
   stream << gdtSelFName << endl;
   stream << ui->timeSpan->textFromValue(ui->timeSpan->value()) << endl;
   for (auto &line : secondPart)  // always same, but has to be there
      stream << line << endl;
   stream << basename + modstr + ".pos" << endl;
   stream << basename + modstr + ".dir" << endl;
   stream << "e" << endl << endl;

   return params;
}

// Read the current gdt file we have in memory and build a list of the chans
bool GravityGui::makeChanList(QString gdt)
{
   bool start_mark = false;
   bool end_mark = false;
   long startTime = 0;
   long endTime = 0;
   QString line;
   int chan, chan_len;
   chanListPair cl_pair;

   currChans.clear();
   analogList.clear();
   ui->neuroChans->clear();
   ui->analogChans->clear();
   ui->selParticles->setText("0");

   QStringList rows = gdt.split('\n',QString::SkipEmptyParts);
   QStringList::const_iterator iter = rows.begin();
   if (*(iter) == "   11 1111111" && *(iter+1) == "   11 1111111")
   {
      iter += 2;
      chan_len = 5;
   }
   else
      chan_len = 2; // if no header, assume a .adt file

    // find first start mark
   for ( ; iter != rows.end(); ++iter)
   {
      line = *iter;
      chan = line.midRef(0,chan_len).toInt(); // 1st number in string is chan
      if (chan == GDT_START)
      {
         start_mark = true;
         ++iter;
         line = *iter;
         startTime = line.mid(chan_len,-1).toInt();
         break;
      }
   }
   if (!start_mark)
      return false;

   for ( ; iter != rows.end(); ++iter)
   {
      line = *iter;
      chan = line.mid(0,chan_len).toInt(); // 1st number in string is chan
      if (chan == GDT_END && start_mark)
      {
         end_mark = true;
         break;
      }
      else
         endTime = line.mid(chan_len,-1).toInt();

      if (chan < 4096)    // neuron channels
      {
         cl_pair = currChans.emplace(chan,0);
         ++(cl_pair.first->second);  // # spikes
      }
      else
      {
         chan /= 4096;
         analogList.insert(chan);
      }
   }
   double elapsed = (endTime - startTime) * (0.5/1000.0);  // seconds
   ui->timeSpan->setValue(elapsed);

   QFont font(ui->neuroChans->font());
   QFontInfo finfo(font);
   int box;
   if (finfo.pixelSize() < 11)
      box = finfo.pixelSize();
   else
      box = finfo.pixelSize()+2;
   QString style;
   QTextStream(&style) << "QCheckBox::indicator { width: " << box << "px; height: " << box << "px;}";
   for (auto &val : currChans)
   {
      QListWidgetItem *item = new QListWidgetItem("",ui->neuroChans);
      QCheckBox *cb = new QCheckBox("Neuron Chan " + QString::number(val.first));
      cb->setChecked(false);
      cb->setProperty("channum",val.first);
      cb->setFont(font);
      cb->setStyleSheet(style);
      ui->neuroChans->setItemWidget(item,cb);
      neuroMapper->setMapping(cb,val.first);
      connect(cb,SIGNAL(clicked()),neuroMapper,SLOT(map()));
   }
   for (auto &val : analogList)
   {
      QListWidgetItem *item = new QListWidgetItem("",ui->analogChans);
      QCheckBox *cb = new QCheckBox("Analog Chan " + QString::number(val));
      cb->setChecked(false);
      cb->setProperty("channum",val);
      cb->setFont(font);
      cb->setStyleSheet(style);
      ui->analogChans->setItemWidget(item,cb);
      analogMapper->setMapping(cb,val);
      connect(cb,SIGNAL(clicked()),analogMapper,SLOT(map()));
   }

   QString msg;
   if (!end_mark)
   {
      QTextStream(&msg) << "Warning: No end marker in file" << endl;
      ui->gbatchTerm->printWarn(msg);
      msg.clear();
   }
   QTextStream(&msg) << "Found " << analogList.size() << " analog channels and " << currChans.size() << " neuron channels" << endl;
   ui->gbatchTerm->append(msg);

   return true;
}


void GravityGui::neuroClicked(int chan)
{
   QObject *obj = neuroMapper->mapping(chan);
   QCheckBox *cb = dynamic_cast<QCheckBox*>(obj);
   if (cb != nullptr)
   {
            // todo: limit to 64 max
      if (cb->checkState() == Qt::Checked)
      {
         selectedChans.insert(chan);
         ui->selParticles->setText(QString::number(selectedChans.size()));
      }
      else
      {
         selectedChans.erase(chan);
         ui->selParticles->setText(QString::number(selectedChans.size()));
      }
      paramsDirty();
   }
}

void GravityGui::analogClicked(int /* chan */)
{

}


void GravityGui::paramsDirty()
{
   dirtyFlag = true;
}
void GravityGui::paramsClean()
{
   dirtyFlag = false;
}

void GravityGui::checkDirty()
{
   if (dirtyFlag)
   {
      QMessageBox msgBox(this);
      msgBox.setText("The parameters have been changed.");
      msgBox.setInformativeText("Do you want to save the changes before proceeding?");
      msgBox.setStandardButtons(QMessageBox::Save);
      msgBox.addButton(tr("Discard Changes"),QMessageBox::NoRole);
      msgBox.setDefaultButton(QMessageBox::Save);
      msgBox.setIcon(QMessageBox::Warning);
      int ret = msgBox.exec();
      if (ret == QMessageBox::Save)
         paramSave();
      paramsClean();  // only get one chance
   }
}

// update other file names based on this
void GravityGui::baseNameChanged()
{
   paramFName = ui->baseName->text() + ui->fnameMod->text() + ".prm";
   ui->paramFullName->setText(paramFName);
   paramsDirty();
}

// update other file names based on this
void GravityGui::modNameChanged()
{
   paramFName = ui->baseName->text() + ui->fnameMod->text() + ".prm";
   ui->paramFullName->setText(paramFName);
   paramsDirty();
}

void GravityGui::setBaseMod(const QString& name)
{
   QString mod;
   QString base = name; // heuristic to find modifier
   int mod_idx = base.lastIndexOf("-");
   if (mod_idx != -1)
   {
     mod = base.right(base.length()-mod_idx);
     base = base.remove(mod_idx,base.length());
   }
   ui->baseName->setText(base);
   ui->fnameMod->setText(mod);
   paramFName = base + mod + ".prm";
   ui->paramFullName->setText(paramFName);
}

bool GravityGui::setSurrogatesArgs(QStringList& args)
{
   QString infile;
   QString tmp;
   QString msg;

   // prepare argument list for invocation
   infile = ui->gdtFullName->text();
   if (infile.length() == 0)
   {
      gbatchSwitch();
      QTextStream(&msg) << tr("You need to load a .gdt file.") << endl;
      ui->gbatchTerm->printWarn(msg);
      msg.clear();
      paramLoad();
      surrogatesSwitch();
      infile = ui->gdtFullName->text(); // user bailed?
      if (infile.length() == 0)
      {
         QTextStream(&msg) << tr("No file was selected, operation abandoned.") << endl;
         ui->surrogatesTerm->printWarn(msg);
         return false;
      }
   }
   args << infile;

   tmp = ui->shiftValues->currentText();
   args << "count" << tmp;

   tmp=ui->surrSeed->text();
   if (tmp.length() != 0)
      args << "seed" << tmp;

   args << "exclude";
   analogSetIter a_iter;
   for (a_iter = analogList.begin(); a_iter != analogList.end(); ++a_iter)
      args << QString::number(*a_iter+1000);
   return true; 
}


void GravityGui::quitCurrentProg()
{
   int curr = ui->termTab->currentIndex();
   switch (curr)
   {
      case TABS::GBATCH:
        if (progGbatch && progGbatch->progIsRunning() != QProcess::NotRunning)
           progGbatch.get()->terminateProg(); 
        break;
      case TABS::XTRYDIS:
        if (progTrydis && progTrydis->progIsRunning() != QProcess::NotRunning)
           progTrydis.get()->terminateProg(); 
        break;
      case TABS::XPROJTM:
        if (progXprojtm && progXprojtm->progIsRunning() != QProcess::NotRunning)
           progXprojtm.get()->terminateProg(); 
        break;
      case TABS::SURROGATES:
        if (progSurrogates && progSurrogates->progIsRunning() != QProcess::NotRunning)
           progSurrogates.get()->terminateProg(); 
        if (progGsig && progGsig->progIsRunning() != QProcess::NotRunning)
           progGsig.get()->terminateProg(); 
        break;
      case TABS::XSLOPE:
        if (progXslope && progXslope->progIsRunning() != QProcess::NotRunning)
           progXslope.get()->terminateProg(); 
        break;
      case TABS::SPKPAT:
        if (progSpkPat && progSpkPat->progIsRunning() != QProcess::NotRunning)
           progSpkPat.get()->terminateProg(); 
        break;
      case TABS::FIREWORKS:
        if (progFireworks && progFireworks->progIsRunning() != QProcess::NotRunning)
           progFireworks.get()->terminateProg(); 
        break;
      case TABS::THREEDJMP:
        if (prog3DJmp && prog3DJmp->progIsRunning() != QProcess::NotRunning)
           prog3DJmp.get()->terminateProg(); 
        break;
      case TABS::DIRECT3D:
        if (prog3d && prog3d->progIsRunning() != QProcess::NotRunning)
           prog3d.get()->terminateProg(); 
        break;
      default:
        break;
   }
}

// Create bdt from edt. This is from edt2bdt.f
bool GravityGui::edt2bdt(QString& edt, QString& bdt)
{
   int chan_len = 5;
   int bdt_t_wid = 8;
   int time, chan;
   QFile e_file(edt);
   QFile b_file(bdt);
   QString line;
   QString msg;

   if (!e_file.open(QIODevice::ReadOnly))
   {
      QTextStream(&msg) << tr("Error opening file ") << edt << endl << tr("Error is: ") << e_file.errorString() << endl;
      ui->gbatchTerm->printWarn(msg);
      ui->gbatchTerm->show();
      return false;
   }

   if (!b_file.open(QIODevice::WriteOnly))
   {
      QTextStream(&msg) << tr("Error opening file ") << bdt << endl << tr("Error is: ") << b_file.errorString() << endl;
      ui->gbatchTerm->printWarn(msg);
      return false;
   }

   e_file.readLine(); // skip header
   e_file.readLine();

   QTextStream stream(&b_file);

   stream << "   11 1111111" << endl; // write header
   stream << "   11 1111111" << endl;
   QApplication::setOverrideCursor(Qt::WaitCursor);
   ui->gbatchTerm->printWarn("Creating .bdt file. This could take a while. . .\n");
   ui->gbatchTerm->repaint();
   while (!e_file.atEnd())
   {
      line = e_file.readLine();
      chan = line.midRef(0,chan_len).toInt();
      time = line.mid(chan_len,-1).toInt();
      time /= 5;  // .1 ms to .5 ms res
      stream.setFieldAlignment(QTextStream::AlignRight);
      stream << qSetFieldWidth(chan_len) << chan << qSetFieldWidth(bdt_t_wid) << time << qSetFieldWidth(0) << endl;
   }
   ui->gbatchTerm->append("Done.\n");
   QApplication::restoreOverrideCursor();
   ui->gbatchTerm->repaint();
   return true;
}

void GravityGui::warnTooLong(const QString& fname)
{
  QString msg;
  QMessageBox msgBox(this);

   QTextStream(&msg) << tr("The file name\n") << fname << tr("\nis longer than ") << GBATCH_MAX_FNAME << tr(" characters.") << endl << tr("The gbatch program requires shorter file names. Please enter a shorter filename or rename an existing file.")<< endl;
  ui->gbatchTerm->printWarn(msg);
  msgBox.setText(msg);
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.setIcon(QMessageBox::Information);
  msgBox.exec();
}

// add (maybe new) dir to the list
void GravityGui::updateRecents()
{
   QString current = ui->currentSession->text();
   if (current.length())
   {
      recentProjs.removeAll(current);        // update list, no dups
      recentProjs.prepend(current);
      while (recentProjs.size() > MAX_RECENTS)
         recentProjs.removeLast();
      rebuildRecents();
   }
}

void GravityGui::removeRecent(const QString& recent)
{
   recentProjs.removeAll(recent);
   for (int entry = 0; entry < MAX_RECENTS; ++entry)
      menuProjs[entry]->setVisible(false);
   rebuildRecents();
}

void GravityGui::rebuildRecents()
{
   int saves, total;

   total = recentProjs.size();
   for (saves = 0; saves < total; ++saves)
   {
      QString line = tr("&%1 %2").arg(saves+1).arg(QFileInfo(recentProjs[saves]).fileName());
      menuProjs[saves]->setText(line);
      menuProjs[saves]->setData(recentProjs[saves]);
      menuProjs[saves]->setVisible(true);
   }
}


// collection to make a program's terminal visible
void GravityGui::gbatchSwitch()
{
  ui->termTab->setCurrentIndex(TABS::GBATCH);
}

void GravityGui::xtrydisSwitch()
{
  ui->termTab->setCurrentIndex(TABS::XTRYDIS);
}

void GravityGui::xprojtmSwitch()
{
  ui->termTab->setCurrentIndex(TABS::XPROJTM);
}

void GravityGui::surrogatesSwitch()
{
  ui->termTab->setCurrentIndex(TABS::SURROGATES);
}

void GravityGui::gsigSwitch()
{
  ui->termTab->setCurrentIndex(TABS::SURROGATES); // share terminal
}

void GravityGui::xslopeSwitch()
{
  ui->termTab->setCurrentIndex(TABS::XSLOPE);
}

void GravityGui::spkPatSwitch()
{
  ui->termTab->setCurrentIndex(TABS::SPKPAT);
}

void GravityGui::threeDJmpSwitch()
{
  ui->termTab->setCurrentIndex(TABS::THREEDJMP);
}

void GravityGui::direct3dSwitch()
{
  ui->termTab->setCurrentIndex(TABS::DIRECT3D);
}

void GravityGui::fireworksSwitch()
{
  ui->termTab->setCurrentIndex(TABS::FIREWORKS);
}
void GravityGui::saveSwitch()
{
  ui->termTab->setCurrentIndex(TABS::SAVE);
}

