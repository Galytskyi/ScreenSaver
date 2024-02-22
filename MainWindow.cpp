#include "MainWindow.h"

#include "Database.h"
#include "Options.h"
#include "OptionsDialog.h"

// -------------------------------------------------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // init interface
    //
    createInterface();

    // open database
    //
    theDatabase.setDatabaseOption(theOptions.database());
    theDatabase.open();
    connect(this, &MainWindow::appendPacket, &theDatabase, &Database::appendToBase, Qt::QueuedConnection);

    // PacketBase
    //
    m_packetBase.load();
    connect(this, &MainWindow::appendPacket, &m_packetBase, &PacketBase::appendToBase, Qt::QueuedConnection);

    // saving thread
    //
    connect(&m_savingThread, &SavingThread::started, this, &MainWindow::onSavingThreadStarted, Qt::QueuedConnection);
    connect(&m_savingThread, &SavingThread::finished, this, &MainWindow::onSavingThreadStoped, Qt::QueuedConnection);
    connect(&m_savingThread, &SavingThread::screenshotComplite, this, &MainWindow::onScreenshotComplite, Qt::QueuedConnection);
}

// -------------------------------------------------------------------------------------------------------------------

MainWindow::~MainWindow()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool MainWindow::createInterface()
{
    setWindowTitle(tr("Screen saver"));

    QRect screen = QGuiApplication::primaryScreen()->availableGeometry();
    resize(static_cast<int>(screen.width() * 0.9), static_cast<int>(screen.height() * 0.9));
    move(screen.center() - rect().center());

    createActions();
    createMenu();
    createToolBars();
    createPanels();
    createPacketView();
    createStatusBar();

    loadSettings();

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createActions()
{
    // Screenshots
    //
    m_pStartSavingAction = new QAction(tr("&Start saving"), this);
    m_pStartSavingAction->setShortcut(QKeySequence{Qt::CTRL | Qt::Key_F5});
    m_pStartSavingAction->setIcon(QIcon(":/icons/Start.png"));
    m_pStartSavingAction->setToolTip(tr("Start saving"));
    connect(m_pStartSavingAction, &QAction::triggered, this, &MainWindow::onStartSaving);

    m_pStopSavingAction = new QAction(tr("&Stop saving..."), this);
    m_pStopSavingAction->setShortcut(QKeySequence{Qt::CTRL | Qt::Key_F6});
    m_pStopSavingAction->setIcon(QIcon(":/icons/Stop.png"));
    m_pStopSavingAction->setToolTip(tr("Stop saving"));
    m_pStopSavingAction->setEnabled(false);
    connect(m_pStopSavingAction, &QAction::triggered, this, &MainWindow::onStopSaving);

    m_pRemoveImageAction = new QAction(tr("&Remove ..."), this);
    m_pRemoveImageAction->setShortcut(QKeySequence{Qt::CTRL | Qt::Key_Delete});
    m_pRemoveImageAction->setIcon(QIcon(":/icons/Remove.png"));
    m_pRemoveImageAction->setToolTip(tr("Remove image"));
    connect(m_pRemoveImageAction, &QAction::triggered, this, &MainWindow::onRemoveImage);

    // View
    //
    m_pPicturesListAction = new QAction(tr("&Pictures ..."), this);
    m_pPicturesListAction->setIcon(QIcon(":/icons/Device.png"));
    m_pPicturesListAction->setToolTip(tr("List of known pictures"));
    connect(m_pPicturesListAction, &QAction::triggered, this, &MainWindow::onPicturesList);

    // Tools
    //
    m_pOptionsAction = new QAction(tr("&Options ..."), this);
    m_pOptionsAction->setShortcut(QKeySequence{Qt::CTRL | Qt::Key_O});
    m_pOptionsAction->setIcon(QIcon(":/icons/Options.png"));
    m_pOptionsAction->setToolTip(tr("Application settings"));
    connect(m_pOptionsAction, &QAction::triggered, this, &MainWindow::onOptions);

    // ?
    //
    m_pAboutQtAction = new QAction(tr("About Qt ..."), this);
    m_pAboutQtAction->setIcon(QIcon(":/icons/About Connection.png"));
    m_pAboutQtAction->setToolTip(tr("Show Qt information"));
    connect(m_pAboutQtAction, &QAction::triggered, this, &MainWindow::OnAboutQt);

    m_pDocumentationAction = new QAction(tr("Documentation ..."), this);
    m_pDocumentationAction->setIcon(QIcon(":/icons/About Connection.png"));
    m_pDocumentationAction->setToolTip(tr("Show documentation of data packets"));
    connect(m_pDocumentationAction, &QAction::triggered, this, &MainWindow::onDocumentation);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createMenu()
{
    QMenuBar* pMenuBar = menuBar();
    if (pMenuBar == nullptr)
    {
        return;
    }

    // Screenshots
    //
    m_pImageMenu = pMenuBar->addMenu(tr("&Screenshots"));

    m_pImageMenu->addAction(m_pStartSavingAction);
    m_pImageMenu->addAction(m_pStopSavingAction);
    m_pImageMenu->addSeparator();
    m_pImageMenu->addAction(m_pRemoveImageAction);

    // View
    //
    m_pViewMenu = pMenuBar->addMenu(tr("&View"));

    m_pViewMenu->addAction(m_pPicturesListAction);
    m_pViewMenu->addSeparator();

    // Tools
    //
    m_pToolsMenu = pMenuBar->addMenu(tr("&Tools"));

    m_pToolsMenu->addAction(m_pOptionsAction);

    // ?
    //
    m_pInfoMenu = pMenuBar->addMenu(tr("&?"));

    m_pInfoMenu->addAction(m_pAboutQtAction);
    m_pInfoMenu->addAction(m_pDocumentationAction);
}

// -------------------------------------------------------------------------------------------------------------------

bool MainWindow::createToolBars()
{
    // Control panel
    //
    m_pControlToolBar = new QToolBar(this);
    if (m_pControlToolBar != nullptr)
    {
        m_pControlToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
        m_pControlToolBar->setWindowTitle(tr("Control panel"));
        m_pControlToolBar->setObjectName(m_pControlToolBar->windowTitle());
        addToolBarBreak(Qt::TopToolBarArea);
        addToolBar(m_pControlToolBar);

        m_pControlToolBar->addAction(m_pStartSavingAction);
        m_pControlToolBar->addAction(m_pStopSavingAction);
        m_pControlToolBar->addSeparator();
        m_pControlToolBar->addAction(m_pRemoveImageAction);
        m_pControlToolBar->addSeparator();
        m_pControlToolBar->addAction(m_pOptionsAction);
    }

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createPanels()
{
    // Picture view panel
    //
    m_pPictureViewPanel = new PanelPictureView(this);
    if (m_pPictureViewPanel != nullptr)
    {
        m_pPictureViewPanel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);

        addDockWidget(Qt::RightDockWidgetArea, m_pPictureViewPanel);

        //m_pPictureViewPanel->hide();

        QAction* previewAction = m_pPictureViewPanel->toggleViewAction();
        if (previewAction != nullptr)
        {
            previewAction->setText(tr("&Picture view panel ..."));
            previewAction->setShortcut(QKeySequence{Qt::CTRL | Qt::Key_I});
            previewAction->setIcon(QIcon(":/icons/Preview.png"));
            previewAction->setToolTip(tr("Picture panel view"));

            if (m_pViewMenu != nullptr)
            {
                m_pViewMenu->addAction(previewAction);
            }

            if (m_pControlToolBar != nullptr)
            {
                //m_pControlToolBar->addAction(previewAction);
            }
        }
    }
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createPacketView()
{
    m_packetView = new PacketView(this);
    if (m_packetView == nullptr)
    {
        return;
    }

    m_packetView->setFrameStyle(QFrame::NoFrame);

    //
    //
    connect(this, &MainWindow::appendPacket, m_packetView, &PacketView::onAppendPacket, Qt::QueuedConnection);
    connect(m_pRemoveImageAction, &QAction::triggered, m_packetView, &PacketView::onRemovePacket, Qt::QueuedConnection);

    connect(m_packetView, &PacketView::updateInBase, &theDatabase, &Database::updateInBase, Qt::QueuedConnection);
    connect(m_packetView, &PacketView::removeFromBase, &theDatabase, &Database::removeFromBase, Qt::QueuedConnection);
    connect(m_packetView, &PacketView::removeFromBase, &m_packetBase, &PacketBase::removeFromBase, Qt::QueuedConnection);

    connect(&m_packetBase, &PacketBase::packetBaseLoaded, m_packetView, &PacketView::onPacketBaseLoaded, Qt::QueuedConnection);

    connect(m_packetView, &PacketView::packetSelected, this, &MainWindow::onPacketCliked, Qt::QueuedConnection);

    //
    //
    setCentralWidget(m_packetView);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createStatusBar()
{
    QStatusBar* pStatusBar = statusBar();
    if (pStatusBar == nullptr)
    {
        return;
    }

    // create
    //
    m_statusEmpty = new QLabel(pStatusBar);
    m_statusSaving = new QLabel(pStatusBar);

    // addWidget
    //
    pStatusBar->addWidget(m_statusSaving);
    pStatusBar->addWidget(m_statusEmpty);

    pStatusBar->setLayoutDirection(Qt::RightToLeft);

    // set default value
    //
    m_statusEmpty->setText(QString());

    onSavingThreadStoped();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onStartSaving()
{
    if (m_savingThread.isRunning() == true)
    {
        QMessageBox::critical(this, windowTitle(), tr("Saving thread is already running"));
        return;
    }

    m_savingThread.start();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onStopSaving()
{
    m_savingThread.stop();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onRemoveImage()
{
    // To see PacketView
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onPicturesList()
{
    QMessageBox::information(this, qAppName(), "Pictures!");
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onOptions()
{
    OptionsDialog dialog(theOptions.time(), theOptions.database(), this);
    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    // save new options
    //
    theOptions.setTime(dialog.timeOption());
    theOptions.time().save();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::OnAboutQt()
{
    QMessageBox::aboutQt(this, qAppName());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onDocumentation()
{
    QString filePath = QApplication::applicationDirPath() + "/docs/Packets.pdf";

    QFile file(filePath);
    if (file.exists() == false)
    {
        QMessageBox::critical(this, qAppName(), QObject::tr("Help file '%1' does not exist!").arg(filePath));
        return;
    }

    QUrl url = QUrl::fromLocalFile(filePath);
    QDesktopServices::openUrl(url);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onPacketCliked(DataPacket* pPacket)
{
    if (m_pPictureViewPanel == nullptr)
    {
        return;
    }

    if (pPacket == nullptr)
    {
        return;
    }

    if (pPacket->packetID() == SQL_INVALID_KEY)
    {
        return;
    }

    if (pPacket->hash() == UNDEFINED_HASH)
    {
         return;
    }

    m_pPictureViewPanel->showPicture(*pPacket);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onSavingThreadStarted()
{
    if (m_pStartSavingAction == nullptr || m_pStopSavingAction == nullptr)
    {
        return;
    }

    if (m_statusSaving == nullptr)
    {
        return;
    }

    m_pStartSavingAction->setEnabled(false);
    m_pStopSavingAction->setEnabled(true);

    m_statusSaving->setText(tr(" Saving every: ") + QString::number(theOptions.time().seconds()) + " sec. ");
    m_statusSaving->setStyleSheet("background-color: rgb(160, 255, 160);");
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onSavingThreadStoped()
{
    if (m_pStartSavingAction == nullptr || m_pStopSavingAction == nullptr)
    {
        return;
    }

    if (m_statusSaving == nullptr)
    {
        return;
    }

    m_pStartSavingAction->setEnabled(true);
    m_pStopSavingAction->setEnabled(false);

    m_statusSaving->setText(tr(" Not saving "));
    m_statusSaving->setStyleSheet("background-color: rgb(255, 160, 160);");

    m_savingThread.setFinishThread(false);
}


// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onScreenshotComplite(DataPacket* pPacket)
{
    if (pPacket == nullptr)
    {
        return;
    }

   DataPacket lastPacket = getLastScreenshot();
   if (lastPacket.imageData().size() == 0)
    {
        pPacket->setCmpPercent(100);
        emit appendPacket(pPacket);
        return;
    }

    // compare Screenshots in the thread
    //
    QFuture<void> resultRun = QtConcurrent::run(MainWindow::comareScreenshots, this, pPacket, lastPacket);
}

// -------------------------------------------------------------------------------------------------------------------

DataPacket MainWindow::getLastScreenshot()
{
    SqlTable* pTable = theDatabase.openTable(SQL_TABLE_PICTURE);
    if (pTable == nullptr)
    {
        return DataPacket();
    }

    if (pTable->open() == false)
    {
        return DataPacket();
    }

    int lastPacketID = pTable->lastKey();
    if (lastPacketID <= 0)
    {
        pTable->close();
        return DataPacket();
    }

    DataPacket packet;

    int readCount = pTable->read(&packet, lastPacketID);
    if (readCount == 0)
    {
        pTable->close();
        return DataPacket();
    }

    pTable->close();

    return packet;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::comareScreenshots(MainWindow* pMainWindow, DataPacket* pNewPacket, DataPacket lastPacket)
{
    if (pMainWindow == nullptr)
    {
        return;
    }

    if (pNewPacket == nullptr)
    {
        return;
    }

    int count = std::max(pNewPacket->imageData().size(), lastPacket.imageData().size());
    if (count == 0)
    {
        return;
    }

    int errors = 0;
    for (int i = 0; i < count; i++)
    {
        if (i >= pNewPacket->imageData().size() || i >= lastPacket.imageData().size())
        {
            errors ++;
            continue;
        }

        if (pNewPacket->imageData().at(i) != lastPacket.imageData().at(i))
        {
            errors++;
        }
    }

    double cmpPercent = errors * 100 / count;

    pNewPacket->setCmpPercent(cmpPercent);

    emit pMainWindow->appendPacket(pNewPacket);
}


// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onContextMenu(QPoint)
{
    if (m_pContextMenu == nullptr)
    {
        return;
    }

    m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::loadSettings()
{
    QSettings s;

    QByteArray geometry = s.value(QString("%1MainWindow/geometry").arg(WINDOW_GEOMETRY_OPTIONS_KEY)).toByteArray();
    QByteArray state = s.value(QString("%1MainWindow/State").arg(WINDOW_GEOMETRY_OPTIONS_KEY)).toByteArray();

    restoreGeometry(geometry);
    restoreState(state);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::saveSettings()
{
    QSettings s;

    s.setValue(QString("%1MainWindow/Geometry").arg(WINDOW_GEOMETRY_OPTIONS_KEY), saveGeometry());
    s.setValue(QString("%1MainWindow/State").arg(WINDOW_GEOMETRY_OPTIONS_KEY), saveState());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent* e)
{
    // stop saving thread before close
    //
    m_savingThread.stop();

    // wait for finish saving thread
    //
    for(int t = 0; t <= 10; t++)
    {
        if (m_savingThread.isFinished() == true)
        {
            break;
        }

        QThread::msleep(SAVING_THREAD_TIMEOUT_STEP);
    }

    // close database
    //
    theDatabase.close();

    //
    //
    saveSettings();

    //
    //
    QMainWindow::closeEvent(e);
}

// -------------------------------------------------------------------------------------------------------------------
