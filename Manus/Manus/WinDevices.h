#pragma once

#include "Devices.h"

#include <functional>
#include <thread>

class WinDevices :
	public Devices
{
private:
	bool m_running;
	std::thread m_thread;

public:
	WinDevices();
	~WinDevices();

private:
	static void DeviceThread(WinDevices* devices);
	static LRESULT CALLBACK WinProcCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};
