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


#include "gravity_gui.h"
#include "g_prog.h"
#include "ui_gravity_gui.h"


#pragma GCC diagnostic ignored "-Wunused-result"
#pragma GCC diagnostic ignored "-Wunused-parameter"

GravityGui::GravityGui(QWidget *parent) : QMainWindow(parent), ui(new Ui::GravityGuiCtls)
{
   ui->setupUi(this);
   setWindowIcon(QIcon(":/gravity-gui.png"));
   QWidget::setWindowIcon(QIcon(":/gravity-gui.png"));
   QString title("GRAVITATIONAL CLUSTERING  Version: ");
   title = title.append(VERSION);
   setWindowTitle(title);

   setupImpl();
}

GravityGui::~GravityGui()
{
   delete ui;
   ui = nullptr;
}

void GravityGui::on_actionQuit_triggered()
{
    actionQuit();
}

void GravityGui::on_xtrydisButton_clicked()
{
  doXtrydis();

}

void GravityGui::on_buttonQuit_clicked()
{
    actionQuit();
}


void GravityGui::on_gdtFile_clicked()
{
   gdtFileOpen();
}

void GravityGui::on_paramLoadFile_clicked()
{
    paramLoad();
}

void GravityGui::on_paramSaveFile_clicked()
{
    paramSave();
}

void GravityGui::on_shiftValues_currentIndexChanged(int index)
{
    paramsDirty();
}

void GravityGui::on_timeStep_valueChanged(double arg1)
{
    paramsDirty();
}

void GravityGui::on_slideValue_textChanged(const QString &arg1)
{
   paramsDirty();
}

void GravityGui::on_normFactor_valueChanged(const QString &arg1)
{
    paramsDirty();
}

void GravityGui::on_acceptorValues_currentIndexChanged(int index)
{
    paramsDirty();
}

void GravityGui::on_effectorValues_currentIndexChanged(int index)
{
    paramsDirty();
}

void GravityGui::on_forceSign_currentTextChanged(const QString &arg1)
{
    paramsDirty();
}

void GravityGui::on_forwardTau_valueChanged(double arg1)
{
    paramsDirty();
}

void GravityGui::on_backwardTau_valueChanged(double arg1)
{
    paramsDirty();
}

void GravityGui::on_forwardInc_valueChanged(double arg1)
{
    paramsDirty();
}

void GravityGui::on_backCharge_valueChanged(double arg1)
{
    paramsDirty();
}

void GravityGui::on_wellDiam_valueChanged(double arg1)
{
    paramsDirty();
}

void GravityGui::on_timeSpan_valueChanged(double arg1)
{
    paramsDirty();
}

void GravityGui::on_gravityOpts_currentIndexChanged(int index)
{
    paramsDirty();
}

void GravityGui::on_gBatch_clicked()
{
    doGbatch();
}

void GravityGui::on_xprojtmButton_clicked()
{
   doXprojtm();

}


void GravityGui::on_actionCreate_GDT_File_triggered()
{
   makeGDT();
}

void GravityGui::on_surrogatesButton_clicked()
{
    doSurrogates();
}

void GravityGui::on_gsigButton_clicked()
{
   doGsig();
}

void GravityGui::on_xslopeButton_clicked()
{
   doXslope();
}

void GravityGui::on_sessionButton_clicked()
{
   doSession();
}

void GravityGui::on_actionHelp_triggered()
{
    doHelp();
}

void GravityGui::on_actionAdjust_Run_Button_Font_triggered()
{
   doRunButtonFont();
}

void GravityGui::on_actionAdjust_Label_Font_triggered()
{
    doLabelFont();
}

void GravityGui::on_actionAdjust_Button_Font_triggered()
{
   doOtherButtonFont();
}

void GravityGui::on_actionAdjust_Input_Controls_Font_triggered()
{
   doInputFont();

}

void GravityGui::on_spkpat6bg_clicked()
{
   doSpkPat("spkpat6bg");
}

void GravityGui::on_spkpat6bgr_clicked()
{
   doSpkPat("spkpat6bgr");
}

void GravityGui::on_spkpat6kbg_clicked()
{
      doSpkPat("spkpat6kbg");
}

void GravityGui::on_spkpatdist_clicked()
{
   doSpkPat("spkpatdist");
}

void GravityGui::on_spkpatwip_clicked()
{
      doSpkPat("spkpatwip");
}

void GravityGui::on_direct3d_bl_clicked()
{
   doDirect3d("direct3d_bl");
}

void GravityGui::on_direct3d_bl_mp_clicked()
{
   doDirect3d("direct3d_bl_mp");
}

void GravityGui::on_direct3d_bl_sig_01_clicked()
{
   doDirect3d("direct3d_bl_sig_01");
}

void GravityGui::on_direct3d_bl_sig_05_clicked()
{
   doDirect3d("direct3d_bl_sig_05");
}

void GravityGui::on_direct3d_bl_sub_clicked()
{
   doDirect3d("direct3d_bl_sub");
}

void GravityGui::on_fireworksButton_clicked()
{
   doFireworks();
}

void GravityGui::on_threeDJmpButton_clicked()
{
   do3DJmp();
}

void GravityGui::on_baseName_textEdited(const QString &/*arg1*/)
{
    baseNameChanged();
}

void GravityGui::on_fnameMod_textEdited(const QString &/* arg1*/)
{
   modNameChanged();
}

void GravityGui::on_quitGProg_clicked()
{
   quitCurrentProg();
}

void GravityGui::on_createGDT_clicked()
{
   makeGDT();
}

void GravityGui::on_winCap_clicked()
{
   doWinCap();
}

void GravityGui::on_openViewer_clicked()
{
   doOpenViewer();
}

void GravityGui::on_actionClear_Recent_Session_List_triggered()
{
   doClearRecents();
}
