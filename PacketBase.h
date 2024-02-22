#ifndef PACKETBASE_H
#define PACKETBASE_H

#include <QObject>
#include <QDateTime>
#include <QMutex>
#include <QMessageBox>
#include <QElapsedTimer>

// ==============================================================================================

#define UNDEFINED_HASH 0x0000000000000000ULL

// ==============================================================================================

#define IMG_PACKET_TIME_FORMAT "dd-MM-yyyy hh:mm:ss"

// ==============================================================================================

const int MAX_IMAGE_DATA_SIZE = 17600;

// ==============================================================================================

class DataPacket
{

public:

	explicit DataPacket();
	virtual ~DataPacket();

public:

	void virtual clear();

	int packetID() const { return m_packetID; }
	void setPacketID(int id) { m_packetID = id; }

    quint64 hash() const { return m_hash; }
    QString hashStr() const;
    void setHash(quint64 hash) { m_hash = hash; }

	QDateTime packetTime() const { return m_packetTime; }
	void setPacketTime(const QDateTime& time) { m_packetTime = time; }

	int imageWidth() const { return m_imageWidth; }
	void setImageWidth(int width) { m_imageWidth = width; }

	int imageHeight() const { return m_imageHeight; }
	void setImageHeight(int height) { m_imageHeight = height; }

    QString resolutionStr() const;

    double cmpPercent() const { return m_cmpPercent; }
    QString cmpPercentStr() const;
    void setCmpPercent(double percent) { m_cmpPercent = percent; }

	QByteArray& imageData() { return m_imageData; }
	void setImageData(const QByteArray& imageData) { m_imageData = imageData; }

private:

	int m_packetID = -1;											// primary key of record in SQL table

    quint64 m_hash = UNDEFINED_HASH;
	QDateTime m_packetTime;											// time when packet was received

	int m_imageWidth = 0;
	int m_imageHeight = 0;

    double m_cmpPercent = 0;

	QByteArray m_imageData;

};

// ==============================================================================================

class PacketBase : public QObject
{
	Q_OBJECT

public:

	explicit PacketBase(QObject *parent = nullptr);
	virtual ~PacketBase() override;

public:

	int count() const;
	void clear();

	int load();

	int append(DataPacket* pPacket);
	DataPacket* packet(int index) const;
	bool remove(int index);
	bool remove( const std::vector<int>& keyList);	// keyList this is list of packetID

private:

	mutable QMutex m_packetMutex;
	std::vector<DataPacket*> m_packetList;

signals:

	void packetBaseLoaded(const std::vector<DataPacket*>& list);

public slots:

	void appendToBase(DataPacket* pPacket);
	void removeFromBase(const std::vector<int>& keyList);
};

// ==============================================================================================

QString timeToStr(const QDateTime& m_packetTime);

// ==============================================================================================


#endif // PACKETBASE_H
