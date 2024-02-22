#ifndef PANELPICTUREVIEW_H
#define PANELPICTUREVIEW_H

#include "PacketBase.h"

#include <QApplication>
#include <QMainWindow>
#include <QDockWidget>

#include <QWidget>
#include <QLabel>
#include <QMenu>
#include <QToolBar>
#include <QTableWidget>
#include <QStatusBar>
#include <QClipboard>
#include <QFileDialog>

// ==============================================================================================

class PanelPictureView : public QDockWidget
{
	Q_OBJECT

public:

	explicit PanelPictureView(QWidget* parent = nullptr);
	virtual ~PanelPictureView() override;

public:

    void showPicture(DataPacket packet);

private:

	QMainWindow* m_pPictureWindow = nullptr;

	QMenu* m_pImageMenu = nullptr;
	QAction* m_pCopyAction = nullptr;
	QAction* m_pSaveAsAction = nullptr;
	QMenu* m_pContextMenu = nullptr;

	QLabel* m_imageLable  = nullptr;
    QPixmap m_pixmap;

	QLabel* m_timeLabel = nullptr;
	QLabel* m_sizeLabel = nullptr;

    DataPacket m_packet;

	void createInterface();
	void createContextMenu();

private slots:

	void onContextMenu(QPoint);

	void copy();
	void saveAs();
};

// ==============================================================================================

#endif // PANELPICTUREVIEW_H
