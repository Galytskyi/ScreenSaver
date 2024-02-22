#include "PacketBase.h"

#include "Database.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DataPacket::DataPacket()
{
	DataPacket::clear();
}

// -------------------------------------------------------------------------------------------------------------------

DataPacket::~DataPacket()
{
}

// -------------------------------------------------------------------------------------------------------------------

void DataPacket::clear()
{
    m_packetID = SQL_INVALID_KEY;

    m_hash = UNDEFINED_HASH;
	m_packetTime.setDate(QDate());

	m_imageWidth = 0;
	m_imageHeight = 0;

    m_cmpPercent = 0;

	m_imageData.clear();
}

// -------------------------------------------------------------------------------------------------------------------

QString DataPacket::hashStr() const
{
    return QString::number(m_hash);
}

// -------------------------------------------------------------------------------------------------------------------

QString DataPacket::resolutionStr() const
{
    return QString::number(m_imageWidth) + " x " + QString::number(m_imageHeight);
}

// -------------------------------------------------------------------------------------------------------------------

QString DataPacket::cmpPercentStr() const
{
    return QString::number(m_cmpPercent, 'f', 2) + " %";
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

PacketBase::PacketBase(QObject *parent)
	: QObject{parent}
{
}

// -------------------------------------------------------------------------------------------------------------------


 PacketBase::~PacketBase()
 {
 }

 // -------------------------------------------------------------------------------------------------------------------

 int PacketBase::count() const
 {
	 QMutexLocker locker(&m_packetMutex);

	 return static_cast<int>(m_packetList.size());
 }

 // -------------------------------------------------------------------------------------------------------------------

 void PacketBase::clear()
 {
	 QMutexLocker locker(&m_packetMutex);

	 qDeleteAll(m_packetList);
 }

 // -------------------------------------------------------------------------------------------------------------------

 int PacketBase::load()
 {
     QElapsedTimer responseTime;
     responseTime.start();

     if (theDatabase.isOpen() == false)
     {
         return false;
     }

     SqlTable* pTable = theDatabase.openTable(SQL_TABLE_PICTURE_INFO);
     if (pTable == nullptr)
     {
         return false;
     }

     m_packetMutex.lock();

         int recordCount = pTable->recordCount();
         if (recordCount > 0)
         {
             m_packetList.resize(static_cast<quint64>(recordCount));

             DataPacket*	pPacketList = new DataPacket[static_cast<quint64>(recordCount)];;
             if (pPacketList != nullptr)
             {
                int readedCount = pTable->read(pPacketList);
                if (readedCount == recordCount)
                {
                    for(int i = 0; i < recordCount; i++)
                    {
                        m_packetList[i] = pPacketList + i;
                    }
                }
                else
                {
                    assert(0);
                    qDebug() << __FUNCTION__ << ": Error of reading" <<
                                " readed: " << readedCount <<
                                " form: " << recordCount;

                    delete [] pPacketList;
                }
             }
         }

     m_packetMutex.unlock();

     pTable->close();

     qDebug() << __FUNCTION__ << ": DataPackets"	<<
                 ", Loaded records: " << count()	<<
                 ", Time for load: " << responseTime.elapsed() << " ms";

     emit packetBaseLoaded(m_packetList);

	 return count();
 }

 // -------------------------------------------------------------------------------------------------------------------

 int PacketBase::append(DataPacket* pPacket)
 {
	 if (pPacket == nullptr)
	 {
		 return -1;
	 }

	 int index = -1;

	 m_packetMutex.lock();

		 m_packetList.push_back(pPacket);
		 index = static_cast<int>(m_packetList.size() - 1);

	 m_packetMutex.unlock();

	 return index;
 }

 // -------------------------------------------------------------------------------------------------------------------

 void PacketBase::appendToBase(DataPacket* pPacket)
 {
	 if (pPacket == nullptr)
	 {
		 return;
	 }

	 int index = append(pPacket);
	 if (index == -1)
	 {
		 QMessageBox::critical(nullptr, tr("Save packets"), tr("Error saving packets to memory"));
		 return;
	 }
 }

 // ----------------------------------------------------------------------------------------------

 DataPacket* PacketBase::packet(int index) const
 {
	 QMutexLocker locker(&m_packetMutex);

	 if (index < 0 || index >= static_cast<int>(m_packetList.size()))
	 {
		 return nullptr;
	 }

	 return m_packetList[static_cast<quint64>(index)];
 }

 // -------------------------------------------------------------------------------------------------------------------

 bool PacketBase::remove(int index)
 {
	 QMutexLocker locker(&m_packetMutex);

	 if (index < 0 || index >= static_cast<int>(m_packetList.size()))
	 {
		 return false;
	 }

	 auto it = m_packetList.cbegin();

	 std::advance(it, index);

//	 auto pPacket = *it;
//	 if (pPacket != nullptr)
//	 {
//		 delete pPacket;
//	 }

	 m_packetList.erase(it);

	 return true;
 }

 // -------------------------------------------------------------------------------------------------------------------

 bool PacketBase::remove(const std::vector<int>& keyList)
 {
	 int keyCount = static_cast<int>(keyList.size());
	 if (keyCount == 0)
	 {
		 return false;
	 }

	 int packetCount = count();
	 if (packetCount == 0)
	 {
		 return false;
	 }

	 int removed = 0;

	 m_packetMutex.lock();

		 for(int packetID : keyList)
		 {
			 auto foundID = std::find_if(m_packetList.begin(), m_packetList.end(), [&packetID](DataPacket* pPacket)
											 {
												 return pPacket->packetID() == packetID;
											 });

			 if (foundID == m_packetList.end())
			 {
				 continue;
			 }

			 m_packetList.erase(foundID);

			 removed++;
		 }

	m_packetMutex.unlock();

	if (removed != keyCount)
	{
		return false;
	}

	return true;
 }

 // -------------------------------------------------------------------------------------------------------------------

 void PacketBase::removeFromBase(const std::vector<int>& keyList)
 {
	 bool result = remove(keyList);
	 if (result == false)
	 {
		 QMessageBox::critical(nullptr, tr("Delete packets"), tr("Error remove packets from memory"));
	 }
 }

 // -------------------------------------------------------------------------------------------------------------------
 // -------------------------------------------------------------------------------------------------------------------
 // -------------------------------------------------------------------------------------------------------------------

 QString timeToStr(const QDateTime& dateTime)
 {
	 QString timeStr;

	 timeStr = QString::asprintf("%02d-%02d-%04d %02d:%02d:%02d",

								 dateTime.date().day(),
								 dateTime.date().month(),
								 dateTime.date().year(),

								 dateTime.time().hour(),
								 dateTime.time().minute(),
								 dateTime.time().second());
	 return timeStr;
 }

 // -------------------------------------------------------------------------------------------------------------------
 // -------------------------------------------------------------------------------------------------------------------
 // -------------------------------------------------------------------------------------------------------------------
