#include "SavingThread.h"

#include "Options.h"
#include "PacketBase.h"

// -------------------------------------------------------------------------------------------------------------------

SavingThread::SavingThread(QObject *parent)
    : QThread{parent}
    , m_bFinishThread{false}
{
}

// -------------------------------------------------------------------------------------------------------------------

SavingThread::~SavingThread()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SavingThread::run()
{
    m_bFinishThread = false;

    while(m_bFinishThread == false)
    {
        createScreenshot();
        waitTimeout();
    }
}

// -------------------------------------------------------------------------------------------------------------------

void SavingThread::stop()
{
    m_bFinishThread = true;
}

// -------------------------------------------------------------------------------------------------------------------

void SavingThread::createScreenshot()
{
    QScreen* pScreen = QGuiApplication::primaryScreen();
    if (pScreen == nullptr)
    {
        return;
    }

    QPixmap pixmap;
    pixmap = pScreen->grabWindow(0);

    DataPacket* pPacket = new DataPacket;
    if (pPacket == nullptr)
    {
        return;
    }

    // time hash
    //
    QDateTime currentTime = QDateTime::currentDateTime();
    QString currentTimeStr = timeToStr(currentTime);
    std::size_t timeHash = std::hash<std::string>{}(currentTimeStr.toStdString());

    pPacket->setHash(timeHash);
    pPacket->setPacketTime(currentTime);

    // width and height
    //
    pPacket->setImageWidth(pixmap.width());
    pPacket->setImageHeight(pixmap.height());

    // data
    //
    QByteArray bArray;
    QBuffer buffer(&bArray);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "BMP");

    pPacket->setImageData(bArray);

    // emit
    //
    emit screenshotComplite(pPacket);
}

// -------------------------------------------------------------------------------------------------------------------

void SavingThread::waitTimeout()
{
    for(int t = 0; t <= (theOptions.time().seconds() * 10); t++)
    {
        if (m_bFinishThread == true)
        {
            break;
        }

        QThread::msleep(SAVING_THREAD_TIMEOUT_STEP);
    }
}

// -------------------------------------------------------------------------------------------------------------------
