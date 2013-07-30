#include "stdafx.h"

#define WND_CLASS_NAME TEXT("yontma")

GUID UsbMassStoreGUID = { 0xeec5ad98, 0x8080, 0x425f,
                          0x92, 0x2a, 0xda, 0xbf, 0x3d, 0xe3, 0xf6, 0x9a};

BOOL InitializeWindowClass();
HWND CreateWindowTracker();
INT_PTR WINAPI WinProcCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL DoRegisterDeviceInterfaceToHwnd(__in GUID InterfaceClassGuid, __in HWND hWnd, __out HDEVNOTIFY *hDeviceNotify);

PMONITOR_THREAD_PARAMS pMonitorThreadParams;

DWORD WINAPI USBDeviceMonitorThread(LPVOID lpParams)
{
    HRESULT hr;
    pMonitorThreadParams = (PMONITOR_THREAD_PARAMS)lpParams;

    WriteLineToLog("USBDeviceMonitorThread: Started");

    if (!InitializeWindowClass())
    {
        goto cleanexit;
    }
    
    HWND hWnd = CreateWindowTracker();
    
    DEV_BROADCAST_DEVICEINTERFACE deviceInterface;
    ZeroMemory(&deviceInterface, sizeof(DEV_BROADCAST_DEVICEINTERFACE));
    deviceInterface.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    deviceInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    deviceInterface.dbcc_classguid = UsbMassStoreGUID;

    HDEVNOTIFY notifyDeviceNode = RegisterDeviceNotification(hWnd, &deviceInterface, DEVICE_NOTIFY_WINDOW_HANDLE);

    MSG msg;
    while(1) {
        if (PeekMessage(&msg, NULL, 0, 0, TRUE))
            DispatchMessage(&msg);

        switch (WaitForSingleObject(pMonitorThreadParams->hMonitorStopEvent, DEFAULT_SLEEP_TIME)) {
        case WAIT_OBJECT_0:
            goto cleanexit;
        case WAIT_TIMEOUT:
            continue;
        }
    }

cleanexit:
    WriteLineToLog("USBDeviceMonitorThread: Exiting");
    InterlockedIncrement(pMonitorThreadParams->pMonitorsCompleted);

    HB_SAFE_FREE(pMonitorThreadParams);

    return 0;
}

BOOL InitializeWindowClass()
{
    WNDCLASSEX wndClass;
    HRESULT result;
    
    ZeroMemory(&wndClass, sizeof(WNDCLASSEX));
    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wndClass.hInstance = (HINSTANCE)GetModuleHandle(0);
    wndClass.lpfnWndProc = (WNDPROC)WinProcCallback;
    wndClass.lpszClassName = WND_CLASS_NAME;
    
    result = RegisterClassEx(&wndClass);

    if (FAILED(result))
    {
        WriteLineToLog("USBDeviceMonitorThread: Failed to initialize window class");
        return FALSE;
    }

    return TRUE;
}

HWND CreateWindowTracker()
{
    HWND hWnd = CreateWindowEx(NULL,
                               WND_CLASS_NAME,
                               TEXT("Dummy Window"),
                               0,
                               0, 0,
                               0, 0,
                               NULL, NULL,
                               NULL,
                               NULL);

    if (hWnd == NULL)
    {
        WriteLineToLog("USBDeviceMonitorThread: Failed to create window");
        return 0;
    }

    return hWnd;
}

INT_PTR WINAPI WinProcCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRet = 1;
    static HDEVNOTIFY hDeviceNotify;
    static HWND hEditWnd;
    static ULONGLONG msgCount = 0;

    switch (message)
    {
    case WM_DEVICECHANGE:
        {
            switch (wParam)
            {
            case DBT_DEVICEARRIVAL:
                WriteLineToLog("USBDeviceMonitorThread: Firing monitor event");
                SetEvent(pMonitorThreadParams->hMonitorEvent);
                break;
            }
        }
        break;
    default:
        // Send all other messages on to the default windows handler.
        lRet = DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return lRet;
}

BOOL DoRegisterDeviceInterfaceToHwnd(__in GUID InterfaceClassGuid, __in HWND hWnd, __out HDEVNOTIFY *hDeviceNotify)
{
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

    ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = InterfaceClassGuid;

    *hDeviceNotify = RegisterDeviceNotification(hWnd,
                                                &NotificationFilter,
                                                DEVICE_NOTIFY_WINDOW_HANDLE);

    if ( NULL == *hDeviceNotify ) 
    {
        return FALSE;
    }

    return TRUE;
}