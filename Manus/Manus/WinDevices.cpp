#include "stdafx.h"
#include "WinDevices.h"

#include <dbt.h>
#include <Hidsdi.h>

WinDevices::WinDevices()
{
	// Register a ManusDevices class
	WNDCLASS wnd_class;
	memset(&wnd_class, 0, sizeof(wnd_class));
	wnd_class.lpfnWndProc = (WNDPROC)WinProcCallback;
	wnd_class.hInstance = GetModuleHandle(0);
	wnd_class.lpszClassName = L"ManusDevices";
	RegisterClass(&wnd_class);

	// Create a message-only window
	hWnd = CreateWindowEx(0, L"ManusDevices", nullptr, 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);

	// Register a device notification broadcast
	DEV_BROADCAST_DEVICEINTERFACE notify_filter;
	memset(&notify_filter, 0, sizeof(notify_filter));
	notify_filter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	notify_filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	HidD_GetHidGuid(&notify_filter.dbcc_classguid);
	device_notify = RegisterDeviceNotification(hWnd, &notify_filter, DEVICE_NOTIFY_WINDOW_HANDLE);
}

WinDevices::~WinDevices()
{
	UnregisterDeviceNotification(device_notify);
	DestroyWindow(hWnd);
}

LRESULT CALLBACK WinDevices::WinProcCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WinDevices* devices = (WinDevices*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (message == WM_DEVICECHANGE && wParam == DBT_DEVICEARRIVAL) {
		PDEV_BROADCAST_DEVICEINTERFACE broadcast = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;

		// Convert the device string to a lower case ASCII device path
		size_t len = wcslen(broadcast->dbcc_name);
		char* device_path = new char[len + 1];
		for (size_t i = 0; i < len; i++)
			device_path[i] = tolower(wctob(broadcast->dbcc_name[i]));
		device_path[len] = '\0'; // Don't forget the terminator

		if (devices->m_connected)
			devices->m_connected(device_path);
		return TRUE;
	}
	else
		return DefWindowProc(hWnd, message, wParam, lParam);
}
