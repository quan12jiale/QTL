#include "systemsemaphore.h"
#include "systemsemaphore_p.h"
#include <windows.h>

/*!
  WINAPI used:

  handle(SystemSemaphore::AccessMode): HANDLE CreateSemaphoreW (LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, LONG lInitialCount, LONG lMaximumCount, LPCWSTR lpName);
  setErrorString(QLatin1String function): DWORD GetLastError (VOID);
  cleanHandle(): WINBOOL CloseHandle (HANDLE hObject);
  modifySemaphore(int count): WINBOOL ReleaseSemaphore (HANDLE hSemaphore, LONG lReleaseCount, LPLONG lpPreviousCount);
                              DWORD WaitForSingleObjectEx (HANDLE hHandle, DWORD dwMilliseconds, WINBOOL bAlertable);
 */

SystemSemaphorePrivate::SystemSemaphorePrivate() :
    semaphore(nullptr), error(SystemSemaphore::NoError)
{
}

Qt::HANDLE SystemSemaphorePrivate::handle(SystemSemaphore::AccessMode)
{
    // don't allow making handles on empty keys
    if (key.isEmpty())
        return nullptr;

    // Create it if it doesn't already exists.
    if (semaphore == nullptr) {
        semaphore = CreateSemaphore(nullptr, initialValue, MAXLONG,
            reinterpret_cast<const wchar_t*>(fileName.utf16()));

        if (semaphore == nullptr)
            setErrorString(QLatin1String("QSystemSemaphore::handle"));
    }

    return semaphore;
}

void SystemSemaphorePrivate::setErrorString(const QString& function)
{
    DWORD windowsError = GetLastError();
    if (windowsError == 0)
        return;

    switch (windowsError) {
    case ERROR_NO_SYSTEM_RESOURCES:
    case ERROR_NOT_ENOUGH_MEMORY:
        error = SystemSemaphore::OutOfResources;
        errorString = QString("%1: out of resources").arg(function);
        break;
    case ERROR_ACCESS_DENIED:
        error = SystemSemaphore::PermissionDenied;
        errorString = QString("%1: permission denied").arg(function);
        break;
    default:
        errorString = QString("%1: unknown error %2").arg(function).arg(windowsError);
        error = SystemSemaphore::UnknownError;
    }
}

void SystemSemaphorePrivate::cleanHandle()
{
    if (semaphore && !CloseHandle(semaphore)) {

    }
    semaphore = nullptr;
}

bool SystemSemaphorePrivate::modifySemaphore(int count)
{
    if (nullptr == handle())
        return false;

    if (count > 0) {
        if (0 == ReleaseSemaphore(semaphore, count, nullptr)) {
            setErrorString(QLatin1String("QSystemSemaphore::modifySemaphore"));
            return false;
        }
    }
    else {
        if (WAIT_OBJECT_0 != WaitForSingleObjectEx(semaphore, INFINITE, FALSE)) {
            setErrorString(QLatin1String("QSystemSemaphore::modifySemaphore"));
            return false;
        }
    }

    clearError();
    return true;
}
