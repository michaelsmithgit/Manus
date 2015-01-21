// Manus.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Manus.h"
#include "Glove.h"
#include "Devices.h"
#include "hidapi.h"

#ifdef _WIN32
#include "WinDevices.h"
#endif

#include <vector>

#define MANUS_VENDOR_ID 0x2341
#define MANUS_PRODUCT_ID 0x8037
#define MANUS_GLOVE_PAGE 0x03
#define MANUS_GLOVE_USAGE 0x04

std::vector<Glove*> g_gloves;
Devices* g_devices;

void DeviceConnected()
{
	for (Glove* glove : g_gloves)
		if (!glove->IsRunning())
			glove->Connect();
}

int ManusInit()
{
	if (hid_init() != 0)
		return MANUS_ERROR;

	// Enumerate the Manus devices on the system
	struct hid_device_info *hid_devices, *current_device;
	hid_devices = hid_enumerate(MANUS_VENDOR_ID, MANUS_PRODUCT_ID);
	current_device = hid_devices;
	for (int i = 0; current_device != nullptr; ++i)
	{
		// The Arduino Micro has two interfaces, the second one contains the input data
		if (current_device->usage_page == MANUS_GLOVE_PAGE && current_device->usage == MANUS_GLOVE_USAGE)
		{
			g_gloves.push_back(new Glove(current_device->path));
		}
		current_device = current_device->next;
	}
	hid_free_enumeration(hid_devices);

#ifdef _WIN32
	g_devices = new WinDevices();
	g_devices->SetDeviceConnected(DeviceConnected);
#endif

	return MANUS_SUCCESS;
}

int ManusExit()
{
	for (Glove* glove : g_gloves)
		delete glove;

	if (hid_exit() != 0)
		return MANUS_ERROR;

#ifdef _WIN32
	delete g_devices;
#endif

	return MANUS_SUCCESS;
}

int ManusGetGloveCount()
{
	return g_gloves.size();
}

int ManusGetState(unsigned int glove, GLOVE_STATE* state, bool euler_angles)
{
	if (glove >= g_gloves.size())
		return MANUS_OUT_OF_RANGE;

	if (!g_gloves[glove]->IsRunning())
		return MANUS_DISCONNECTED;

	if (!state)
		return MANUS_INVALID_ARGUMENT;

	return g_gloves[glove]->GetState(state, euler_angles) ? MANUS_SUCCESS : MANUS_ERROR;
}

int ManusEnableGamepad(bool enabled)
{
	return MANUS_ERROR;
}

int ManusEnableKeyboard(bool enabled)
{
	return MANUS_ERROR;
}

int ManusEnableMouse(bool enabled)
{
	return MANUS_ERROR;
}
