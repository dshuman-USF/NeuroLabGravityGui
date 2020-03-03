#ifndef GRAVITY_GUI_H
#define GRAVITY_GUI_H

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

#include <QMainWindow>
#include <QProcess>
#include <QTextStream>
#include <QSignalMapper>
#include <QCheckBox>
#include <QColor>
#include <QStringList>
#include <QAction>
#include <set>
#include <memory>
#include "ReplWidget.h"
//#include "g_prog.h"

using namespace std;

// First part of file is fixed length
enum PARAMS1 {SHIFT=0,OUTFILE,PARTICLES,TIMESTEP,SLIDE,NORM,ACCEPTOR,EFFECTOR,
             FORCE,RATE_NORM,FWD_TAU,BCK_TAU,FWD_CHG,BCK_CHG,WELL_DIAM,OPTIONS,P1_END};
// Second part depends on # of particles. The second part is fixed offset
// after you add in the # of particles. Use this, e.g., as #particles+RESPONSE
enum PARAMS2 {RESPONSE=0,INFILE,TIMESPAN};

// the second part of the file is unused, constant, but has to be there.
// These are the default values:
const QStringList secondPart({"N", "2","2","1","1","1","1","2","1000",
                              "11"});

enum TABS {GBATCH=0,XTRYDIS,XPROJTM,SURROGATES,XSLOPE,SPKPAT,FIREWORKS,THREEDJMP,DIRECT3D,SAVE};

enum FTYPE {ADT=0,BDT,EDT};
const QString capDir("captures");
const int MAX_RECENTS = 8;

using chanList = map<int,int>;
using chanListIter = chanList::iterator;
using chanListPair = pair<chanListIter,bool>;

using selChanList = set<int>;
using selChanListIter = selChanList::iterator;

using analogSet = set<int>;
using analogSetIter = analogSet::iterator;

const int GDT_START=21;
const int GDT_END=22;
const int MAX_SPIKES=10000;
const int PARAM_LINES=32;  // at least this many lines
// a lot of the fortran programs expect short filenames. Warn if a name is too big.
const int GBATCH_MAX_FNAME=30;

// the names of window titles that we know

const QStringList knownProgs={"xtrydis","xprojtm","direct3d","xslope","3djmp","xfire"};

namespace Ui {
class GravityGuiCtls;
}

class GravityProg;

class GravityGui : public QMainWindow
{
    Q_OBJECT
    friend GravityProg;

public:
    explicit GravityGui(QWidget *parent = 0);
    ~GravityGui();

private slots:
    void on_actionQuit_triggered();
    void on_xtrydisButton_clicked();
    void on_buttonQuit_clicked();
    void on_gdtFile_clicked();
    void on_paramLoadFile_clicked();
    void on_paramSaveFile_clicked();
    void neuroClicked(int);
    void analogClicked(int);
    void on_shiftValues_currentIndexChanged(int index);
    void on_timeStep_valueChanged(double arg1);
    void on_slideValue_textChanged(const QString &arg1);
    void on_normFactor_valueChanged(const QString &arg1);
    void on_acceptorValues_currentIndexChanged(int index);
    void on_effectorValues_currentIndexChanged(int index);
    void on_forceSign_currentTextChanged(const QString &arg1);
    void on_forwardTau_valueChanged(double arg1);
    void on_backwardTau_valueChanged(double arg1);
    void on_forwardInc_valueChanged(double arg1);
    void on_backCharge_valueChanged(double arg1);
    void on_wellDiam_valueChanged(double arg1);
    void on_timeSpan_valueChanged(double arg1);
    void on_gravityOpts_currentIndexChanged(int index);
    void on_gBatch_clicked();
    void on_xprojtmButton_clicked();
    void on_actionCreate_GDT_File_triggered();
    void on_surrogatesButton_clicked();
    void on_gsigButton_clicked();
    void on_xslopeButton_clicked();
    void on_sessionButton_clicked();
    void on_actionHelp_triggered();
    void on_actionAdjust_Button_Font_triggered();
    void on_actionAdjust_Label_Font_triggered();
    void on_actionAdjust_Run_Button_Font_triggered();
    void on_actionAdjust_Input_Controls_Font_triggered();
    void on_spkpat6bg_clicked();
    void on_spkpat6bgr_clicked();
    void on_spkpat6kbg_clicked();
    void on_spkpatdist_clicked();
    void on_spkpatwip_clicked();
    void on_direct3d_bl_clicked();
    void on_direct3d_bl_mp_clicked();
    void on_direct3d_bl_sig_01_clicked();
    void on_direct3d_bl_sig_05_clicked();
    void on_direct3d_bl_sub_clicked();
    void on_fireworksButton_clicked();
    void on_threeDJmpButton_clicked();
    void on_baseName_textEdited(const QString &arg1);
    void on_fnameMod_textEdited(const QString &arg1);
    void on_quitGProg_clicked();
    void on_createGDT_clicked();
    void on_winCap_clicked();
    void on_openViewer_clicked();
    void OpenRecentProj();
    void on_actionClear_Recent_Session_List_triggered();

public slots:
    void progGbatchDone(int,QProcess::ExitStatus);
    void progXtrydisDone(int,QProcess::ExitStatus);
    void progXtrydisGotLine(QByteArray);
    void progXprojtmDone(int,QProcess::ExitStatus);
    void progXprojtmGotLine(QByteArray);
    void progSurrogatesDone(int,QProcess::ExitStatus);
    void progGsigDone(int,QProcess::ExitStatus);
    void progXslopeDone(int,QProcess::ExitStatus);
    void progXslopeGotLine(QByteArray);
    void progSpkPatDone(int,QProcess::ExitStatus);
    void progSpkPatGotLine(QByteArray);
    void prog3dDone(int,QProcess::ExitStatus);
    void prog3dGotLine(QByteArray);
    void progFireworksDone(int,QProcess::ExitStatus);
    void progFireworksGotLine(QByteArray);
    void prog3DJmpDone(int,QProcess::ExitStatus);
    void prog3DJmpGotLine(QByteArray);

protected:
      void closeEvent(QCloseEvent *evt);

private:
    void setupImpl();
    void saveSettings();
    void loadSettings();
    void setRunButtonFont(const QFont&);
    void setLabelFont(const QFont&);
    void setOtherButtonFont(const QFont&);
    void setInputFont(const QFont&);
    void makeGDT();
    void warnTooLong(const QString&);
    void makeOffsetsGnew();
    void gdtFileOpen();
    void gdtFileLoad(QString);
    void checkSelected();
    void validateGDT();
    void paramLoad();
    void paramSave();
    QString buildParams();
    void initParams();
    void actionQuit();
    void doClearRecents();
    void rebuildRecents();
    void removeRecent(const QString &);
    void doSession();
    void doWinCap();
    void doOpenViewer();
    void quitCurrentProg();
    void doXtrydis();
    void doGbatch();
    void doXprojtm();
    void doSurrogates();
    void doGsig();
    void doXslope();
    void doSpkPat(QString);
    void doDirect3d(QString);
    void doFireworks();
    void do3DJmp();
    void doRunButtonFont();
    void doLabelFont();
    void doOtherButtonFont();
    void doInputFont();
    void doHelp();
    void baseNameChanged();
    void modNameChanged();
    bool setSurrogatesArgs(QStringList&);
    bool makeChanList(QString);
    void paramsDirty();
    void paramsClean();
    void checkDirty();
    void setBaseMod(const QString&);
    void xtrydisSwitch();
    void gbatchSwitch();
    void xprojtmSwitch();
    void surrogatesSwitch();
    void gsigSwitch();
    void xslopeSwitch();
    void spkPatSwitch();
    void direct3dSwitch();
    void fireworksSwitch();
    void threeDJmpSwitch();
    void saveSwitch();
    bool edt2bdt(QString&, QString&);
    void createCapture();
    void updateRecents();

    QString gdtSelFName;
    QString gdtParamFName;
    QString paramFName;
    chanList currChans;
    analogSet analogList;
    selChanList selectedChans;
    bool haveGDT=false;
    bool dirtyFlag=false;
    int waitForWinTime=0;
    QStringList recentProjs;
    QAction *menuProjs[MAX_RECENTS];

    unique_ptr<GravityProg> progTrydis;
    unique_ptr<GravityProg> progGbatch;
    unique_ptr<GravityProg> progXprojtm;
    unique_ptr<GravityProg> progSurrogates;
    unique_ptr<GravityProg> progGsig;
    unique_ptr<GravityProg> progXslope;
    unique_ptr<GravityProg> progSpkPat;
    unique_ptr<GravityProg> progFireworks;
    unique_ptr<GravityProg> prog3DJmp;
    unique_ptr<GravityProg> prog3d;

    QColor tabBlack = QColor(0,0,0);
    QColor tabRunning = QColor(150,70,0);


    QSignalMapper *neuroMapper;
    QSignalMapper *analogMapper;

    Ui::GravityGuiCtls *ui;
};

#endif // GRAVITY_GUI_H
