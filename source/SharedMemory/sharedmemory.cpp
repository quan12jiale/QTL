#include "sharedmemory.h"
#include "sharedmemory_p.h"
#include "systemsemaphore.h"

SharedMemory::SharedMemory()
    : d(new SharedMemoryPrivate)
{
}

SharedMemory::SharedMemory(const QString& key)
    : d(new SharedMemoryPrivate)
{
    setKey(key);
}

SharedMemory::~SharedMemory()
{
    setKey(QString());
}

void SharedMemory::setKey(const QString& key)
{
    if (key == d->key && d->makePlatformSafeKey(key) == d->nativeKey)
        return;

    if (isAttached())
        detach();
    d->cleanHandle();
    d->key = key;
    d->nativeKey = d->makePlatformSafeKey(key);
}

QString SharedMemory::key() const
{
    return d->key;
}

void SharedMemory::setNativeKey(const QString& key)
{
    if (key == d->nativeKey && d->key.isNull())
        return;

    if (isAttached())
        detach();
    d->cleanHandle();
    d->key = QString();
    d->nativeKey = key;
}

QString SharedMemory::nativeKey() const
{
    return d->nativeKey;
}

bool SharedMemory::create(int size, AccessMode mode)
{
    if (!d->initKey())
        return false;

    QString function = QLatin1String("QSharedMemory::create");
    SharedMemoryLocker lock(this);
    if (!d->key.isNull() && !d->tryLocker(&lock, function))
        return false;


    if (size <= 0) {
        d->error = SharedMemory::InvalidSize;
        d->errorString =
            QString("%1: create size is less then 0").arg(function);
        return false;
    }

    if (!d->create(size))
        return false;

    return d->attach(mode);
}

int SharedMemory::size() const
{
    return d->size;
}

bool SharedMemory::attach(AccessMode mode)
{
    if (isAttached() || !d->initKey())
        return false;

    SharedMemoryLocker lock(this);
    if (!d->key.isNull() && !d->tryLocker(&lock, QLatin1String("QSharedMemory::attach")))
        return false;

    if (isAttached() || !d->handle())
        return false;

    return d->attach(mode);
}

bool SharedMemory::isAttached() const
{
    return (nullptr != d->memory);
}

bool SharedMemory::detach()
{
    if (!isAttached())
        return false;

    SharedMemoryLocker lock(this);
    if (!d->key.isNull() && !d->tryLocker(&lock, QLatin1String("QSharedMemory::detach")))
        return false;

    return d->detach();
}

void* SharedMemory::data()
{
    return d->memory;
}

const void* SharedMemory::constData() const
{
    return d->memory;
}

const void* SharedMemory::data() const
{
    return d->memory;
}

bool SharedMemory::lock()
{
    if (d->lockedByMe) {
        qWarning("QSharedMemory::lock: already locked");
        return true;
    }
    if (d->systemSemaphore.acquire()) {
        d->lockedByMe = true;
        return true;
    }
    QString function = QLatin1String("QSharedMemory::lock");
    d->errorString = QString("%1: unable to lock").arg(function);
    d->error = SharedMemory::LockError;
    return false;
}

bool SharedMemory::unlock()
{
    if (!d->lockedByMe)
        return false;
    d->lockedByMe = false;
    if (d->systemSemaphore.release())
        return true;
    QString function = QLatin1String("QSharedMemory::unlock");
    d->errorString = QString("%1: unable to unlock").arg(function);
    d->error = SharedMemory::LockError;
    return false;
}

SharedMemory::SharedMemoryError SharedMemory::error() const
{
    return d->error;
}

QString SharedMemory::errorString() const
{
    return d->errorString;
}
