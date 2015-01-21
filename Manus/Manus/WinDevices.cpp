#include "stdafx.h"
#include "WinDevices.h"

#include <dbt.h>
#include <Hidsdi.h>

WinDevices::WinDevices()
{
	m_thread = std::thread(DeviceThread, this);
}

WinDevices::~WinDevices()
{
	m_running = false;
	if (m_thread.joinable())
		m_thread.join();
}

void WinDevices::DeviceThread(WinDevices* devices)
{
	devices->m_running = true;

	// Register a ManusDevices class
	WNDCLASS wnd_class;
	memset(&wnd_class, 0, sizeof(wnd_class));
	wnd_class.lpfnWndProc = (WNDPROC)WinProcCallback;
	wnd_class.hInstance = GetModuleHandle(0);
	wnd_class.lpszClassName = L"ManusDevices";
	RegisterClass(&wnd_class);

	// Create a message-only window
	HWND hWnd = CreateWindowEx(0, L"ManusDevices", nullptr, 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)devices);
	
	// Register a device notification broadcast
	HDEVNOTIFY device_notify;
	DEV_BROADCAST_DEVICEINTERFACE notify_filter;
	memset(&notify_filter, 0, sizeof(notify_filter));
	notify_filter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	notify_filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	HidD_GetHidGuid(&notify_filter.dbcc_classguid);
	device_notify = RegisterDeviceNotification(hWnd, &notify_filter, DEVICE_NOTIFY_WINDOW_HANDLE);

	// Get all messages for the window that belongs to this thread.
	// TODO: Message type filtering.
	MSG msg;
	while (devices->m_running && GetMessage(&msg, hWnd, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	devices->m_running = false;
	UnregisterDeviceNotification(device_notify);
	DestroyWindow(hWnd);
}

LRESULT CALLBACK WinDevices::WinProcCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WinDevices* devices = (WinDevices*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (message == WM_DEVICECHANGE && wParam == DBT_DEVICEARRIVAL) {
		if (devices->m_connected)
			devices->m_connected();
		return TRUE;
	}
	else
		return DefWindowProc(hWnd, message, wParam, lParam);
}
