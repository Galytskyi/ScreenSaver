#include "PacketView.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

PacketModel::PacketModel(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

PacketModel::~PacketModel()
{
	QMutexLocker l(&m_packetMutex);

	m_packetList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int PacketModel::columnCount(const QModelIndex&) const
{
	return PACKET_VIEW_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int PacketModel::rowCount(const QModelIndex&) const
{
	return static_cast<int>(m_packetCount);
}

// -------------------------------------------------------------------------------------------------------------------

QVariant PacketModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < PACKET_VIEW_COLUMN_COUNT)
		{
			result = PacketViewColumn[section];
		}
	}

	if (orientation == Qt::Vertical)
	{
		result = QString("%1").arg(section + 1);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant PacketModel::data(const QModelIndex &index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	int rowIndex = index.row();
	if (rowIndex < 0 || rowIndex >= static_cast<int>(m_packetCount))
	{
		return QVariant();
	}

	int columnIndex = index.column();
	if (columnIndex < 0 || columnIndex > PACKET_VIEW_COLUMN_COUNT)
	{
		return QVariant();
	}

	DataPacket* pPacket = at(rowIndex);
	if (pPacket == nullptr)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		int result = Qt::AlignLeft;

		switch (columnIndex)
		{
            case PACKET_VIEW_COLUMN_TIME:		result = Qt::AlignLeft;     break;
            case PACKET_VIEW_COLUMN_HASH:		result = Qt::AlignCenter;	break;
            case PACKET_VIEW_COLUMN_PERCENT:	result = Qt::AlignCenter;	break;
            case PACKET_VIEW_COLUMN_RESOLUTION: result = Qt::AlignCenter;	break;

			default:
				assert(0);
		}

		return result;
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(rowIndex, columnIndex, pPacket);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString PacketModel::text(int row, int column, DataPacket* pPacket) const
{
	if (row < 0 || row >= static_cast<int>(m_packetCount))
	{
		return QString();
	}

	if (column < 0 || column > PACKET_VIEW_COLUMN_COUNT)
	{
		return QString();
	}

	if (pPacket == nullptr)
	{
		return QString();
	}

	QString result;

	switch (column)
	{
        case PACKET_VIEW_COLUMN_TIME:		result = timeToStr(pPacket->packetTime());					break;
        case PACKET_VIEW_COLUMN_HASH:		result = pPacket->hashStr();                                break;
        case PACKET_VIEW_COLUMN_PERCENT:	result = pPacket->cmpPercentStr();                          break;
        case PACKET_VIEW_COLUMN_RESOLUTION: result = pPacket->resolutionStr();                          break;

		default:
			assert(0);
	}

	return result;

}

// -------------------------------------------------------------------------------------------------------------------

bool PacketModel::append(DataPacket* pPacket)
{
	if (pPacket == nullptr)
	{
		return false;
	}

	// append into PacketList
	//
	int indexTable = static_cast<int>(m_packetCount);

	beginInsertRows(QModelIndex(), indexTable, indexTable);

		m_packetMutex.lock();

			m_packetList.push_back(pPacket);
			m_packetCount = m_packetList.size();

		m_packetMutex.unlock();

	endInsertRows();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

DataPacket* PacketModel::at(int index) const
{
	QMutexLocker l(&m_packetMutex);

	if (index < 0 || index >= static_cast<int>(m_packetCount))
	{
		return nullptr;
	}

	return m_packetList[static_cast<quint64>(index)];
}

// -------------------------------------------------------------------------------------------------------------------

void PacketModel::remove(const std::vector<int>& removeIndexList)
{
	// remove from PacketList
	//
	int count = static_cast<int>(removeIndexList.size());
	for(int index = count-1; index >= 0; index--)
	{
		int removeIndex = removeIndexList.at(static_cast<quint64>(index));

		DataPacket* pPacket = at(removeIndex);
		if (pPacket == nullptr)
		{
			continue;
		}

		beginRemoveRows(QModelIndex(), removeIndex, removeIndex);

			m_packetMutex.lock();

				m_packetList.erase(m_packetList.begin() + removeIndex);
				m_packetCount = m_packetList.size();

			m_packetMutex.unlock();

		endRemoveRows();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void PacketModel::set(const std::vector<DataPacket*>& list_add)
{
	quint64 count = list_add.size();
	if (count == 0)
	{
		return;
	}

	beginInsertRows(QModelIndex(), 0, static_cast<int>(count - 1));

		m_packetMutex.lock();

			m_packetList = list_add;
			m_packetCount = m_packetList.size();

		m_packetMutex.unlock();

	endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void PacketModel::clear()
{
	quint64 count = m_packetCount;
	if (count == 0)
	{
		return;
	}

	beginRemoveRows(QModelIndex(), 0, static_cast<int>(count - 1));

		m_packetMutex.lock();

			m_packetList.clear();
			m_packetCount = m_packetList.size();

		m_packetMutex.unlock();

	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

PacketView::PacketView(QWidget* parent) :
	QTableView(parent)
{
	setModel(&m_model);

	connect(this, &PacketView::clicked, this, &PacketView::onPacketListCliked, Qt::QueuedConnection);

	setSelectionBehavior(QAbstractItemView::SelectRows);
	setWordWrap(false);

	QSize cellSize = QFontMetrics(font()).size(Qt::TextSingleLine,"A");
	verticalHeader()->setDefaultSectionSize(cellSize.height());

	for(int column = 0; column < PACKET_VIEW_COLUMN_COUNT; column++)
	{
		setColumnWidth(column, PacketViewWidth[column]);
	}

	createHeaderContextMenu();

	installEventFilter(this);

	createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

PacketView::~PacketView()
{
}

// -------------------------------------------------------------------------------------------------------------------

void PacketView::createContextMenu()
{
	// create context menu
	//
	m_pContextMenu = new QMenu(tr("&Packets"), this);

	m_pContextMenu->addSeparator();

	m_pFindAction = m_pContextMenu->addAction(tr("&Find"));
	m_pFindAction->setIcon(QIcon(":/icons/Find.png"));

	m_pContextMenu->addSeparator();

	m_pCopyAction = m_pContextMenu->addAction(tr("&Copy"));
	m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));

	m_pSelectAllAction = m_pContextMenu->addAction(tr("Select &All"));
	m_pSelectAllAction->setIcon(QIcon(":/icons/SelectAll.png"));

	m_pContextMenu->addSeparator();

	m_pRemoveImageAction = m_pContextMenu->addAction(tr("&Remove ..."));
	m_pRemoveImageAction->setIcon(QIcon(":/icons/Remove.png"));

	connect(m_pCopyAction, &QAction::triggered, this, &PacketView::onCopy, Qt::QueuedConnection);
	connect(m_pFindAction, &QAction::triggered, this, &PacketView::onFind, Qt::QueuedConnection);
	connect(m_pSelectAllAction, &QAction::triggered, this, &PacketView::onSelectAll, Qt::QueuedConnection);
	connect(m_pRemoveImageAction, &QAction::triggered, this, &PacketView::onRemovePacket, Qt::QueuedConnection);

	// init context menu
	//
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &QTableView::customContextMenuRequested, this, &PacketView::onContextMenu, Qt::QueuedConnection);
}

// -------------------------------------------------------------------------------------------------------------------

void PacketView::createHeaderContextMenu()
{
	m_headerContextMenu = new QMenu(this);
	if (m_headerContextMenu == nullptr)
	{
		return;
	}

	// init header context menu
	//
	horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &PacketView::onHeaderContextMenu);

	for(int column = 0; column < PACKET_VIEW_COLUMN_COUNT; column++)
	{
        QAction* pAction = m_headerContextMenu->addAction(PacketViewColumn[column]);
		if (pAction == nullptr)
		{
			continue;
		}

		pAction->setCheckable(true);
		pAction->setChecked(true);

		m_pColumnActionList.push_back(pAction);
	}

	connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered),	this, &PacketView::onColumnAction);

    hideColumn1(PACKET_VIEW_COLUMN_RESOLUTION, true);
}

// -------------------------------------------------------------------------------------------------------------------

void PacketView::hideColumn1(int column, bool hide)
{
	int columnCount = static_cast<int>(m_pColumnActionList.size());
	if (column < 0 || column >= columnCount)
	{
		return;
	}

	if (hide == true)
	{
		hideColumn(column);
		m_pColumnActionList[static_cast<quint64>(column)]->setChecked(false);
	}
	else
	{
		showColumn(column);
		m_pColumnActionList[static_cast<quint64>(column)]->setChecked(true);
	}
}

// -------------------------------------------------------------------------------------------------------------------

int PacketView::firstVisibleColumn()
{
	int visibleColumn = 0;

	int columnCount = model()->columnCount();
	for(int column = 0; column < columnCount; column++)
	{
		if (isColumnHidden(column) == true)
		{
			continue;
		}

		visibleColumn = column;

		break;
	}

	return visibleColumn;
}

// -------------------------------------------------------------------------------------------------------------------

bool PacketView::eventFilter(QObject* object, QEvent* e)
{
	if (e->type() == QEvent::KeyRelease)
	{
		QKeyEvent* ke = static_cast<QKeyEvent* >(e);

		if (	ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down ||
				ke->key() == Qt::Key_PageUp || ke->key() == Qt::Key_PageDown)
		{
			onPacketListCliked(QModelIndex());
		}
	}

	return QTableView::eventFilter(object, e);
}

// -------------------------------------------------------------------------------------------------------------------

void PacketView::onPacketBaseLoaded(const std::vector<DataPacket*>& list_add)
{
	m_model.clear();
	m_model.set(list_add);

	scrollToBottom();

	setCurrentIndex(model()->index(m_model.count() - 1, firstVisibleColumn()));
	onPacketListCliked(QModelIndex());
}

// -------------------------------------------------------------------------------------------------------------------

void PacketView::onAppendPacket(DataPacket* pPacket)
{
	if (pPacket == nullptr)
	{
		return;
	}

	// append into PacketTable
	//
	if (m_model.append(pPacket) == false)
	{
		return;
	}

	setCurrentIndex(model()->index(m_model.count() - 1, firstVisibleColumn()));
	onPacketListCliked(QModelIndex());
}

// -------------------------------------------------------------------------------------------------------------------

void PacketView::onRemovePacket()
{
	int packetCount = m_model.count();
	if (packetCount == 0)
	{
		return;
	}

	std::vector<int> keyList;
	std::vector<int> removeIndexList;

	const QModelIndexList selectedList = selectionModel()->selectedRows();
	for(auto selectedIndex : selectedList)
	{
		int index = selectedIndex.row();
		if (index < 0 || index >= m_model.count())
		{
			continue;
		}

		DataPacket* packet = m_model.at(index);
		if (packet == nullptr)
		{
			continue;
		}

		keyList.push_back(packet->packetID());

		removeIndexList.push_back(index);
	}

	if (removeIndexList.size() == 0)
	{
		return;
	}

	if (QMessageBox::question(this,
							  windowTitle(),
							  tr("Do you want delete %1 packet(s)?").
							  arg(removeIndexList.size())) == QMessageBox::No)
	{
		return;
	}

	std::sort(removeIndexList.begin(), removeIndexList.end());

	// remove from PacketTable
	// remove from PacketView
	//
	m_model.remove(removeIndexList);

	// remove from:
	// - Database
	// - PacketBase
	//
	emit removeFromBase(keyList);
}

// -------------------------------------------------------------------------------------------------------------------

void PacketView::onContextMenu(const QPoint&)
{
	if (m_pContextMenu == nullptr)
	{
		return;
	}

    //m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void PacketView::onFind()
{
//	FindData* dialog = new FindData(this);
//	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void PacketView::onCopy()
{
//	CopyData copyData(this, false);
//	copyData.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void PacketView::onSelectAll()
{
	selectAll();
}

// -------------------------------------------------------------------------------------------------------------------

void PacketView::onPacketListCliked(const QModelIndex &index)
{
	Q_UNUSED(index)

	int indexPacket = currentIndex().row();
	if (indexPacket < 0 || indexPacket >= m_model.count())
	{
		return;
	}

	DataPacket* pPacket = m_model.at(indexPacket);
	if (pPacket == nullptr)
	{
		return;
	}

	emit packetSelected(pPacket);
}

// -------------------------------------------------------------------------------------------------------------------

void PacketView::onHeaderContextMenu(QPoint)
{
	if (m_headerContextMenu == nullptr)
	{
		return;
	}

	m_headerContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void PacketView::onColumnAction(QAction* action)
{
	if (action == nullptr)
	{
		return;
	}

	int columnCount = static_cast<int>(m_pColumnActionList.size());
	for(int column = 0; column < columnCount; column++)
	{
		if (m_pColumnActionList[static_cast<quint64>(column)] == action)
		{
			hideColumn1(column, !action->isChecked());

			break;
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
