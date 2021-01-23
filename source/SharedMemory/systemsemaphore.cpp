#include "systemsemaphore.h"
#include "systemsemaphore_p.h"

SystemSemaphore::SystemSemaphore(const QString& key, int initialValue, AccessMode mode)
    : d(new SystemSemaphorePrivate)
{
    setKey(key, initialValue, mode);
}

SystemSemaphore::~SystemSemaphore()
{
    d->cleanHandle();
}

void SystemSemaphore::setKey(const QString& key, int initialValue, AccessMode mode)
{
    if (key == d->key && mode == Open)
        return;
    d->clearError();

    d->cleanHandle();
    d->key = key;
    d->initialValue = initialValue;
    // cache the file name so it doesn't have to be generated all the time.
    d->fileName = d->makeKeyFileName();
    d->handle(mode);
}

QString SystemSemaphore::key() const
{
    return d->key;
}

bool SystemSemaphore::acquire()
{
    return d->modifySemaphore(-1);
}

bool SystemSemaphore::release(int n)
{
    if (n == 0)
        return true;
    if (n < 0) {
        qWarning("QSystemSemaphore::release: n is negative.");
        return false;
    }
    return d->modifySemaphore(n);
}

SystemSemaphore::SystemSemaphoreError SystemSemaphore::error() const
{
    return d->error;
}

QString SystemSemaphore::errorString() const
{
    return d->errorString;
}
