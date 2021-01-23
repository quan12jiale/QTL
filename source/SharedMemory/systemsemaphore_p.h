#pragma once
#include "systemsemaphore.h"
#include "sharedmemory_p.h"

class SystemSemaphorePrivate
{

public:
    SystemSemaphorePrivate();

    QString makeKeyFileName()
    {
        return SharedMemoryPrivate::makePlatformSafeKey(key, QLatin1String("qipc_systemsem_"));
    }

    inline void setError(SystemSemaphore::SystemSemaphoreError e, const QString& message)
    {
        error = e; errorString = message;
    }
    inline void clearError()
    {
        setError(SystemSemaphore::NoError, QString());
    }

    Qt::HANDLE handle(SystemSemaphore::AccessMode mode = SystemSemaphore::Open);
    void setErrorString(const QString& function);

    void cleanHandle();
    bool modifySemaphore(int count);

    QString key;
    QString fileName;
    int initialValue;

    Qt::HANDLE semaphore;
    //Qt::HANDLE semaphoreLock;

    QString errorString;
    SystemSemaphore::SystemSemaphoreError error;
};
