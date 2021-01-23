#include "sharedmemory.h"
#include "sharedmemory_p.h"
#include "systemsemaphore.h"
#include <QtCore/QCryptographicHash>
#include <windows.h>

/*!
  WINAPI used:

  create(int size): HANDLE CreateFileMappingW (HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCWSTR lpName);
  handle(): HANDLE OpenFileMappingW (DWORD dwDesiredAccess, WINBOOL bInheritHandle, LPCWSTR lpName);
  cleanHandle(): WINBOOL CloseHandle (HANDLE hObject);
  attach(SharedMemory::AccessMode mode): LPVOID MapViewOfFile (HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap);
                                         SIZE_T VirtualQuery (LPCVOID lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, SIZE_T dwLength);
  detach(): WINBOOL UnmapViewOfFile (LPCVOID lpBaseAddress);
  setErrorString(QLatin1String function): DWORD GetLastError (VOID);
 */

SharedMemoryPrivate::SharedMemoryPrivate() : 
    memory(nullptr), size(0), error(SharedMemory::NoError),
    systemSemaphore(QString()), lockedByMe(false), hand(nullptr)
{

}

QString SharedMemoryPrivate::makePlatformSafeKey(const QString& key,
    const QString& prefix)
{
    if (key.isEmpty())
        return QString();

    QString result = prefix;

    for (QChar ch : key) {
        if ((ch >= QLatin1Char('a') && ch <= QLatin1Char('z')) ||
            (ch >= QLatin1Char('A') && ch <= QLatin1Char('Z')))
            result += ch;
    }

    QByteArray hex = QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Sha1).toHex();
    result.append(QLatin1String(hex));
    return result;
}

Qt::HANDLE SharedMemoryPrivate::handle()
{
    if (!hand) {
        const QLatin1String function("QSharedMemory::handle");
        if (nativeKey.isEmpty()) {
            error = SharedMemory::KeyError;
            errorString = QString("%1: unable to make key").arg(function);
            return nullptr;
        }

        hand = OpenFileMapping(FILE_MAP_ALL_ACCESS, false,
            reinterpret_cast<const wchar_t*>(nativeKey.utf16()));

        if (!hand) {
            setErrorString(function);
            return nullptr;
        }
    }
    return hand;
}

bool SharedMemoryPrivate::initKey()
{
    if (!cleanHandle())
        return false;

    systemSemaphore.setKey(QString(), 1);
    systemSemaphore.setKey(key, 1);
    if (systemSemaphore.error() != SystemSemaphore::NoError) {
        QString function = QLatin1String("QSharedMemoryPrivate::initKey");
        errorString = QString("%1: unable to set key on lock").arg(function);
        switch (systemSemaphore.error()) {
        case SystemSemaphore::PermissionDenied:
            error = SharedMemory::PermissionDenied;
            break;
        case SystemSemaphore::KeyError:
            error = SharedMemory::KeyError;
            break;
        case SystemSemaphore::AlreadyExists:
            error = SharedMemory::AlreadyExists;
            break;
        case SystemSemaphore::NotFound:
            error = SharedMemory::NotFound;
            break;
        case SystemSemaphore::OutOfResources:
            error = SharedMemory::OutOfResources;
            break;
        case SystemSemaphore::UnknownError:
        default:
            error = SharedMemory::UnknownError;
            break;
        }
        return false;
    }

    errorString = QString();
    error = SharedMemory::NoError;
    return true;
}

bool SharedMemoryPrivate::cleanHandle()
{
    if (hand != nullptr && !CloseHandle(hand)) {
        hand = nullptr;
        setErrorString(QLatin1String("QSharedMemory::cleanHandle"));
        return false;
    }
    hand = nullptr;
    return true;
}

bool SharedMemoryPrivate::create(int size)
{
    const QLatin1String function("QSharedMemory::create");
    if (nativeKey.isEmpty()) {
        error = SharedMemory::KeyError;
        errorString = QString("%1: key error").arg(function);
        return false;
    }

    // Create the file mapping.
    hand = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, size,
        reinterpret_cast<const wchar_t*>(nativeKey.utf16()));

    setErrorString(function);

    // hand is valid when it already exists unlike unix so explicitly check
    return error != SharedMemory::AlreadyExists && hand;
}

bool SharedMemoryPrivate::attach(SharedMemory::AccessMode mode)
{
    // Grab a pointer to the memory block
    DWORD permissions = (mode == SharedMemory::ReadOnly ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS);
    memory = MapViewOfFile(handle(), permissions, 0, 0, 0);

    if (nullptr == memory) {
        setErrorString(QLatin1String("QSharedMemory::attach"));
        cleanHandle();
        return false;
    }

    // Grab the size of the memory we have been given (a multiple of 4K on windows)
    MEMORY_BASIC_INFORMATION info;
    if (!VirtualQuery(memory, &info, sizeof(info))) {
        // Windows doesn't set an error code on this one,
        // it should only be a kernel memory error.
        error = SharedMemory::UnknownError;
        errorString = QString("%1: size query failed").arg(QLatin1String("QSharedMemory::attach: "));
        return false;
    }
    size = static_cast<int>(info.RegionSize);

    return true;
}

bool SharedMemoryPrivate::detach()
{
    // umap memory
    if (!UnmapViewOfFile(memory)) {
        setErrorString(QLatin1String("QSharedMemory::detach"));
        return false;
    }
    memory = nullptr;
    size = 0;

    // close handle
    return cleanHandle();
}

void SharedMemoryPrivate::setErrorString(QLatin1String function)
{
    DWORD windowsError = GetLastError();
    if (windowsError == 0)
        return;
    switch (windowsError) {
    case ERROR_ALREADY_EXISTS:
        error = SharedMemory::AlreadyExists;
        errorString = QString("%1: already exists").arg(function);
        break;
    case ERROR_FILE_NOT_FOUND:
        error = SharedMemory::NotFound;
        errorString = QString("%1: doesn't exist").arg(function);
        break;
    case ERROR_COMMITMENT_LIMIT:
        error = SharedMemory::InvalidSize;
        errorString = QString("%1: invalid size").arg(function);
        break;
    case ERROR_NO_SYSTEM_RESOURCES:
    case ERROR_NOT_ENOUGH_MEMORY:
        error = SharedMemory::OutOfResources;
        errorString = QString("%1: out of resources").arg(function);
        break;
    case ERROR_ACCESS_DENIED:
        error = SharedMemory::PermissionDenied;
        errorString = QString("%1: permission denied").arg(function);
        break;
    default:
        errorString = QString("%1: unknown error %2").arg(function).arg(windowsError);
        error = SharedMemory::UnknownError;
    }
}
