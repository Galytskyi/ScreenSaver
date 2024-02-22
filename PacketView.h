#ifndef PACKETVIEW_H
#define PACKETVIEW_H

#include <QTableView>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QEvent>
#include <QKeyEvent>

#include "PacketBase.h"

// ==============================================================================================

const char* const			PacketViewColumn[] =
{
                            QT_TRANSLATE_NOOP("PacketView.h", "Time"),
                            QT_TRANSLATE_NOOP("PacketView.h", "Hash"),
                            QT_TRANSLATE_NOOP("PacketView.h", "%"),
                            QT_TRANSLATE_NOOP("PacketView.h", "Resolution"),
};

const int					PACKET_VIEW_COLUMN_COUNT	 = sizeof(PacketViewColumn)/sizeof(PacketViewColumn[0]);

const int                   PACKET_VIEW_COLUMN_TIME         = 0,
                            PACKET_VIEW_COLUMN_HASH         = 1,
                            PACKET_VIEW_COLUMN_PERCENT      = 2,
                            PACKET_VIEW_COLUMN_RESOLUTION   = 3;

const int					PacketViewWidth[PACKET_VIEW_COLUMN_COUNT] =
{
                            150,	// PACKET_VIEW_COLUMN_TIME
                            100,	// PACKET_VIEW_COLUMN_HASH
                            150,	// PACKET_VIEW_COLUMN_PERCENT
                            150,	// PACKET_VIEW_COLUMN_RESOLUTION
};

// ----------------------------------------------------------------------------------------------

class PacketModel : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit PacketModel(QObject* parent = nullptr);
	virtual ~PacketModel() override;

public:

	int count() const { return static_cast<int>(m_packetCount); }

	QString text(int row, int column, DataPacket* pPacket) const;

	bool append(DataPacket* pPacket);
	DataPacket* at(int index) const;
	void remove(const std::vector<int>& removeIndexList);

	void set(const std::vector<DataPacket*>& list_add);
	void clear();

private:

	mutable QMutex m_packetMutex;
	std::vector<DataPacket*> m_packetList;
	quint64 m_packetCount = 0;

	int columnCount(const QModelIndex &parent) const override;
	int rowCount(const QModelIndex &parent=QModelIndex()) const override;

	QVariant headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const override;
	QVariant data(const QModelIndex &index, int role) const override;
};

// ==============================================================================================

class PacketView : public QTableView
{
	Q_OBJECT

public:

	explicit PacketView(QWidget* parent = nullptr);
	virtual ~PacketView() override;

private:

	//
	//
	QMenu* m_pContextMenu = nullptr;

	QAction* m_pRemoveImageAction = nullptr;

	QAction* m_pCopyAction = nullptr;
	QAction* m_pFindAction = nullptr;
	QAction* m_pSelectAllAction = nullptr;

	QMenu* m_headerContextMenu = nullptr;
	std::vector<QAction*> m_pColumnActionList;

	//
	//
	PacketModel m_model;

	void createContextMenu();

	void createHeaderContextMenu();
	void hideColumn1(int column, bool hide);
	int firstVisibleColumn();

protected:

	bool eventFilter(QObject* object, QEvent* e) override;

signals:

	void removeFromBase(const std::vector<int>& keyList);
	void updateInBase(const std::vector<DataPacket*>& list);

	void packetSelected(DataPacket* pPacket);

public slots:

	//
	//
	void onPacketBaseLoaded(const std::vector<DataPacket*>& list_add);

	void onAppendPacket(DataPacket* pPacket);
	void onRemovePacket();

	// slots of menu
	//
	void onContextMenu(const QPoint&);

	void onFind();
	void onCopy();
	void onSelectAll();

	// Slot of Click
	//
	void onPacketListCliked(const QModelIndex& index);

	// slots for list header, to hide or show columns
	//
	void onHeaderContextMenu(QPoint);
	void onColumnAction(QAction* action);
};

// ==============================================================================================

#endif // PACKETVIEW_H
