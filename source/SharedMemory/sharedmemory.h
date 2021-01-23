#pragma once
#include <QtCore/QString>
#include <QtCore/QScopedPointer>

class SharedMemoryPrivate;

class SharedMemory
{
public:
    enum AccessMode
    {
        ReadOnly,
        ReadWrite
    };

    enum SharedMemoryError
    {
        NoError,
        PermissionDenied,
        InvalidSize,
        KeyError,
        AlreadyExists,
        NotFound,
        LockError,
        OutOfResources,
        UnknownError
    };

    SharedMemory();
    SharedMemory(const QString& key);
    ~SharedMemory();

    void setKey(const QString& key);
    QString key() const;
    void setNativeKey(const QString& key);
    QString nativeKey() const;

    bool create(int size, AccessMode mode = ReadWrite);
    int size() const;

    bool attach(AccessMode mode = ReadWrite);
    bool isAttached() const;
    bool detach();

    void* data();
    const void* constData() const;
    const void* data() const;

    bool lock();
    bool unlock();

    SharedMemoryError error() const;
    QString errorString() const;

private:
    QScopedPointer<SharedMemoryPrivate> d;
};
