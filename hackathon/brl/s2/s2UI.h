

#ifndef S2UI_H
#define S2UI_H

#include <QDialog>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QTcpSocket>
#include <QDir>
#include <QCheckBox>
#include <QTabWidget>
#include <QDir>
#include <v3d_interface.h>
#include "s2Controller.h"
#include "s2plot.h"
#include "stackAnalyzer.h"
#include "noteTaker.h"
QT_BEGIN_NAMESPACE
class QWidget;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTcpSocket;
class QNetworkSession;

QT_END_NAMESPACE

//! [0]
class S2UI : public QDialog
{
    Q_OBJECT

public:
    S2UI(V3DPluginCallback2 &callback, QWidget *parent = 0 );
    QLineEdit *s2LineEdit;

    StackAnalyzer *myStackAnalyzer;
public slots:
    void pmStatusHandler(bool pmStatus);
    void handleNewLocation(LandmarkList newlandmarks);
    void loadLatest();
signals:
    void startPM();
    void stopPM();
    void callSALoad(QString);
    void newImageData(Image4DSimple);
    void moveToNext(LocationSimple);
	void noteStatus(QString);
    void processSmartScanSig(QString);
private slots:
    void startS2();
    void startScan();
    void loadScan();
    void displayScan();
    void posMonButtonClicked();
    void updateS2Data(  QMap<int, S2Parameter> currentParameterMap);
    void updateString(QString broadcastedString);
    void updateFileString(QString inputString);
    void startingSmartScan();
    void startingZStack();
    void updateROIPlot(QString ignore);
    void s2ROIMonitor();
    void moveToROI(LocationSimple);
    void clearROIPlot();
    void smartScanHandler();
    QString getFileString();
    void loadScanFromFile(QString fileString);
    void toLoad();
	void status(QString statString);
	void updateLocalRemote(bool state);
    void runSAStuffClicked();
private:
    V3DPluginCallback2 * cb;

//    QThread *workerThread;

 // Layout and buttons

    QTabWidget * lhTabs;
    QTabWidget * rhTabs;


    QGridLayout *mainLayout;
    QGridLayout * gl;

    QDialogButtonBox *buttonBox1;

    QGroupBox *parameterBox;
    QVBoxLayout *vbox;
    QGroupBox *roiGroupBox;
    QGroupBox *createROIMonitor();
    QRectF roiRect;
    QGraphicsScene * roiGS;
    QGraphicsView * roiGV;
    QGraphicsRectItem *newRect;

    QGroupBox *createROIControls();
    QGroupBox *createS2Monitors();
    QGroupBox *createTracingParameters();
    void createButtonBox1();
	
	QCheckBox *localRemoteCB;

    QLabel *s2Label;
    QPushButton *startS2PushButton;
    QPushButton *startScanPushButton;
    QPushButton  *startZStackPushButton;
    QPushButton *loadScanPushButton;
    QPushButton *startPosMonButton;
    QPushButton *startSmartScanPB;
    QPushButton *centerGalvosPB ;
    QPushButton * startStackAnalyzerPB;
    QPushButton *roiClearPB;
    QPushButton *runSAStuff;
    QLineEdit *roiXEdit;
    QLineEdit *roiYEdit;
    QLineEdit *roiZEdit;
    QLineEdit *roiXWEdit ;
    QLineEdit *roiYWEdit ;
    QLineEdit *roiZWEdit;

// External S2 controllers

    S2Controller myController;
    S2Controller myPosMon;


// gui hookup

    void hookUpSignalsAndSlots();
    QDir getSaveDirectory();

//  internal methods and important parameters
    void checkParameters(QMap<int, S2Parameter> currentParameterMap);
    bool isDuplicateROI(LocationSimple inputLocation);

    LandmarkList *allROILocations;
    QMap<int, S2Parameter> uiS2ParameterMap;

    QString fileString ;
    QString lastFile;
    bool posMonStatus;
    bool waitingForFile;
	bool isLocal;
    int smartScanStatus;
    double scanNumber;
    int resultNumber;
    LandmarkList scanList;

    Image4DSimple*  total4DImage;

    NoteTaker *myNotes;

    QDir saveDir;
    QDir currentDirectory;
    QFile saveTextFile;
    QTextStream outputStream;

};
//! [0]

#endif
