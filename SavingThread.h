#ifndef SAVINGTHREAD_H
#define SAVINGTHREAD_H

#include <QThread>
#include <QGuiApplication>
#include <QScreen>
#include <QPixmap>
#include <QBuffer>
#include <QCryptographicHash>

#include "Database.h"
#include "PacketBase.h"

// ==============================================================================================

const int SAVING_THREAD_TIMEOUT_STEP = 100; // 100 milliseconds

// ==============================================================================================

class SavingThread : public QThread
{
    Q_OBJECT

public:

    explicit SavingThread(QObject *parent = nullptr);
    virtual ~SavingThread() override;

    void stop();
    void setFinishThread(bool state) { m_bFinishThread = state; }

protected:

    void run() override;

private:

    std::atomic_bool m_bFinishThread;

    void createScreenshot();
    void waitTimeout();

signals:

    void screenshotComplite(DataPacket*);
};

// ==============================================================================================

#endif // SAVINGTHREAD_H
