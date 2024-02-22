#include "PanelPictureView.h"

#include "Database.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

PanelPictureView::PanelPictureView(QWidget* parent) :
	QDockWidget(parent)
{
	setWindowTitle(tr("Picture view panel"));
	setObjectName(windowTitle());

	createInterface();
	createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

PanelPictureView::~PanelPictureView()
{
}

// -------------------------------------------------------------------------------------------------------------------

void PanelPictureView::createInterface()
{
	m_pPictureWindow = new QMainWindow;
	if (m_pPictureWindow == nullptr)
	{
		return;
	}

	//
	//
	m_pCopyAction = new QAction(tr("&Copy"), m_pPictureWindow);
	m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));
	connect(m_pCopyAction, &QAction::triggered, this, &PanelPictureView::copy, Qt::QueuedConnection);

	m_pSaveAsAction = new QAction(tr("Save &As"), m_pPictureWindow);
	m_pSaveAsAction->setIcon(QIcon(":/icons/Save.png"));
	connect(m_pSaveAsAction, &QAction::triggered, this, &PanelPictureView::saveAs, Qt::QueuedConnection);

	//
	//
	QToolBar* toolBar = new QToolBar(m_pPictureWindow);

	toolBar->addAction(m_pCopyAction);
	toolBar->addAction(m_pSaveAsAction);

	toolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
	toolBar->setWindowTitle(tr("Search measurements ToolBar"));
	m_pPictureWindow->addToolBarBreak(Qt::TopToolBarArea);
	m_pPictureWindow->addToolBar(toolBar);

	//
	//
	m_imageLable = new QLabel(m_pPictureWindow);
	if (m_imageLable == nullptr)
	{
		return;
	}

	//
	//
	QStatusBar* statusBar = m_pPictureWindow->statusBar();
	m_timeLabel = new QLabel(QString(), statusBar);
	m_sizeLabel = new QLabel(QString(), statusBar);

	statusBar->addWidget(m_sizeLabel);
	statusBar->addWidget(m_timeLabel);
	statusBar->setLayoutDirection(Qt::RightToLeft);

	//
	//
	m_pPictureWindow->setCentralWidget(m_imageLable);

	setWidget(m_pPictureWindow);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelPictureView::createContextMenu()
{
	if (m_pPictureWindow == nullptr)
	{
		return;
	}

	// create context menu
	//
	m_pContextMenu = new QMenu(tr("&Pictures"), m_pPictureWindow);

	m_pContextMenu->addAction(m_pCopyAction);
	m_pContextMenu->addSeparator();
	m_pContextMenu->addAction(m_pSaveAsAction);

	// init context menu
	//
	m_imageLable->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_imageLable, &QTableWidget::customContextMenuRequested, this, &PanelPictureView::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelPictureView::showPicture(DataPacket packet)
{
    if (m_imageLable == nullptr)
    {
        return;
    }

    if (packet.packetID() == SQL_INVALID_KEY)
    {
        return;
    }

    m_imageLable->clear();
    m_sizeLabel->setText(QString());
    m_timeLabel->setText(QString());

    //
    //
    SqlTable* pTable = theDatabase.openTable(SQL_TABLE_PICTURE);
    if (pTable == nullptr)
    {
        return;
    }

    if (pTable->open() == false)
    {
        return;
    }

    int readCount = pTable->read(&packet, packet.packetID());
    if (readCount == 0)
    {
        pTable->close();
        return;
    }

    pTable->close();

    //
    //
    if (packet.imageData().size() == 0)
    {
        return;
    }

    bool result = m_pixmap.loadFromData(packet.imageData());
    if (result == false)
    {
        return;
    }

    //
    //
    QPixmap pixmap = m_pixmap.transformed(QTransform().scale(0.3, 0.3));

    m_imageLable->setPixmap(pixmap);
    m_imageLable->setScaledContents(true);
    m_timeLabel->setText(" " + timeToStr(packet.packetTime()) + " ");
    m_sizeLabel->setText(QString(" %1 x %2 ").arg(m_pixmap.width()).arg(m_pixmap.height()));

    m_packet = packet;
}

// -------------------------------------------------------------------------------------------------------------------

void PanelPictureView::onContextMenu(QPoint)
{
	if (m_pContextMenu == nullptr)
	{
		return;
	}

	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void PanelPictureView::copy()
{
	QClipboard* clipboard = QApplication::clipboard();
	if (clipboard == nullptr)
	{
		return;
	}

	clipboard->setPixmap(m_pixmap);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelPictureView::saveAs()
{
    QString filter = tr("JPG files (*.jpg);;BMP files (*.bmp);;PNG files (*.png)");

    QString fileName = timeToStr(m_packet.packetTime()).replace(":", "-");

    fileName = QFileDialog::getSaveFileName(this,
                                                    qApp->translate("PanelImageView", "Save as ..."),
                                                    fileName,
                                                    filter);
    if (fileName.isEmpty() == true)
    {
        return;
    }

    m_pixmap.save(fileName);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
