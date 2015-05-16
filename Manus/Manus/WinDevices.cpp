/**
 * Copyright (C) 2015 Manus Machina
 *
 * This file is part of the Manus SDK.
 * 
 * Manus SDK is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Manus SDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with Manus SDK. If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdafx.h"
#include "Glove.h"
#include "WinDevices.h"

#include <dbt.h>

WinDevices::WinDevices()
{
	m_thread = CreateThread(NULL, 0, WinDevices::DeviceThread, this, 0, &m_thread_id);
}

WinDevices::~WinDevices()
{
	m_running = false;
	PostThreadMessage(m_thread_id, WM_QUIT, 0, 0);
	WaitForSingleObject(m_thread, INFINITE);
	CloseHandle(m_thread);
}

DWORD WINAPI WinDevices::DeviceThread(LPVOID param)
{
	WinDevices* devices = (WinDevices*)param;
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
	notify_filter.dbcc_classguid = GUID_MANUS_GLOVE_SERVICE;
	device_notify = RegisterDeviceNotification(hWnd, &notify_filter, DEVICE_NOTIFY_WINDOW_HANDLE);

	// Get all messages for the window that belongs to this thread.
	// TODO: Message type filtering.
	MSG msg;
	while (devices->m_running && GetMessage(&msg, nullptr, 0, 0) != 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	devices->m_running = false;
	UnregisterDeviceNotification(device_notify);
	DestroyWindow(hWnd);

	return 0;
}

LRESULT CALLBACK WinDevices::WinProcCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WinDevices* devices = (WinDevices*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (message == WM_DEVICECHANGE && wParam == DBT_DEVICEARRIVAL) {
		PDEV_BROADCAST_DEVICEINTERFACE broadcast = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;

		if (devices->m_connected)
			devices->m_connected(broadcast->dbcc_name);
		return TRUE;
	}
	else
		return DefWindowProc(hWnd, message, wParam, lParam);
}
