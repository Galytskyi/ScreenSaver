#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QLabel>
#include <QSettings>
#include <QGuiApplication>
#include <QScreen>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QDesktopServices>
#include <QtConcurrent>

#include "PacketView.h"
#include "PanelPictureView.h"
#include "SavingThread.h"

// ==============================================================================================

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow()  override;

private:

    PacketBase				m_packetBase;
    SavingThread			m_savingThread;

private:

    // Actions of main menu
    //
    // menu - Image
    //
    QAction*				m_pStartSavingAction = nullptr;
    QAction*				m_pStopSavingAction = nullptr;
    QAction*				m_pRemoveImageAction = nullptr;

    // menu - View
    //
    QAction*				m_pPicturesListAction = nullptr;

    // menu - Tools
    //
    QAction*				m_pOptionsAction = nullptr;

    // menu - ?
    //
    QAction*				m_pAboutQtAction = nullptr;
    QAction*				m_pDocumentationAction = nullptr;

    //
    //
    QMenu*					m_pContextMenu = nullptr;

private:

    // Elements of interface - Menu
    //
    QMenu*					m_pImageMenu = nullptr;
    QMenu*					m_pViewMenu = nullptr;
    QMenu*					m_pToolsMenu = nullptr;
    QMenu*					m_pInfoMenu = nullptr;

    // Elements of interface - ToolBar
    //
    QToolBar*				m_pControlToolBar = nullptr;

    // Elements of interface - View
    //
    PacketView*				m_packetView = nullptr;

    // Elements of interface - Panels
    //
    PanelPictureView*		m_pPictureViewPanel = nullptr;

    // Elements of interface - StatusBar
    //
    QLabel*					m_statusEmpty = nullptr;
    QLabel*					m_statusSaving = nullptr;

private:

    bool					createInterface();

    void					createActions();
    void					createMenu();
    bool					createToolBars();
    void					createPanels();
    void					createPacketView();
    void					createStatusBar();

    void					loadSettings();
    void					saveSettings();

private:

    DataPacket              getLastScreenshot();
    static void             comareScreenshots(MainWindow* pMainWindow, DataPacket* pNewPacket, DataPacket lastPacket);

protected:

    void					closeEvent(QCloseEvent* e) override;

signals:

    // from packetReceived
    //
    void					appendPacket(DataPacket*);

private slots:

    // Slots of main menu
    //

    // menu - Image
    //
    void					onStartSaving();
    void					onStopSaving();
    void					onRemoveImage();


    // menu - View
    //
    void					onPicturesList();

    // menu - Tools
    //
    void					onOptions();

    // menu - ?
    //
    void					OnAboutQt();
    void					onDocumentation();

    // Slot of PacketView
    //
    void					onPacketCliked(DataPacket* pPacket);

    // Slots of measure thread
    //
    void					onSavingThreadStarted();
    void					onSavingThreadStoped();
    void					onScreenshotComplite(DataPacket* pPacket);

    // Slots of contex menu
    //
    void					onContextMenu(QPoint);
};

// ==============================================================================================

#endif // MAINWINDOW_H
