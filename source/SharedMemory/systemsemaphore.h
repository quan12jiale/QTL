#pragma once
#include <QtCore/QString>
#include <QtCore/QScopedPointer>

class SystemSemaphorePrivate;

class SystemSemaphore
{
public:
    enum AccessMode
    {
        Open,
        Create
    };

    enum SystemSemaphoreError
    {
        NoError,
        PermissionDenied,
        KeyError,
        AlreadyExists,
        NotFound,
        OutOfResources,
        UnknownError
    };

    SystemSemaphore(const QString& key, int initialValue = 0, AccessMode mode = Open);
    ~SystemSemaphore();

    void setKey(const QString& key, int initialValue = 0, AccessMode mode = Open);
    QString key() const;

    bool acquire();
    bool release(int n = 1);

    SystemSemaphoreError error() const;
    QString errorString() const;

private:
    QScopedPointer<SystemSemaphorePrivate> d;
};
