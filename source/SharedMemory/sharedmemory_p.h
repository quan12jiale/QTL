#pragma once
#include "sharedmemory.h"
#include "systemsemaphore.h"

class SharedMemoryLocker
{
public:
    inline SharedMemoryLocker(SharedMemory* sharedMemory) : q_sm(sharedMemory)
    {
        Q_ASSERT(q_sm);
    }

    inline ~SharedMemoryLocker()
    {
        if (q_sm)
            q_sm->unlock();
    }

    inline bool lock()
    {
        if (q_sm && q_sm->lock())
            return true;
        q_sm = nullptr;
        return false;
    }

private:
    SharedMemory* q_sm;
};


class SharedMemoryPrivate
{
public:
    SharedMemoryPrivate();

    void* memory;
    int size;
    QString key;
    QString nativeKey;
    SharedMemory::SharedMemoryError error;
    QString errorString;

    SystemSemaphore systemSemaphore;
    bool lockedByMe;

    //static int createUnixKeyFile(const QString& fileName);
    static QString makePlatformSafeKey(const QString& key,
        const QString& prefix = QLatin1String("qipc_sharedmemory_"));
    Qt::HANDLE handle();

    bool initKey();
    bool cleanHandle();
    bool create(int size);
    bool attach(SharedMemory::AccessMode mode);
    bool detach();

    void setErrorString(QLatin1String function);

    bool tryLocker(SharedMemoryLocker* locker, const QString& function) {
        if (!locker->lock()) {
            errorString = QString("%1: unable to lock").arg(function);
            error = SharedMemory::LockError;
            return false;
        }
        return true;
    }

private:
    Qt::HANDLE hand;
};