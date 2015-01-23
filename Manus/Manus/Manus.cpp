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
#include <mutex>

#define MANUS_VENDOR_ID 0x2341
#define MANUS_PRODUCT_ID 0x8037
#define MANUS_GLOVE_PAGE 0x03
#define MANUS_GLOVE_USAGE 0x04

std::vector<Glove*> g_gloves;
std::mutex g_gloves_mutex;

Devices* g_devices;

void DeviceConnected(const char* device_path)
{
	// Check if the glove already exists
	for (Glove* glove : g_gloves)
	{
		if (strcmp(device_path, glove->GetDevicePath()) == 0)
		{
			// The glove was previously connected, reconnect it
			glove->Connect();
			return;
		}
	}

	std::lock_guard<std::mutex> lock(g_gloves_mutex);
	struct hid_device_info *hid_device = hid_enumerate_device(device_path);

	// The glove hasn't been connected before, add it to the list of gloves
	if (hid_device->usage_page == MANUS_GLOVE_PAGE && hid_device->usage == MANUS_GLOVE_USAGE)
		g_gloves.push_back(new Glove(device_path));

	hid_free_enumeration(hid_device);
}

int ManusInit()
{
	if (hid_init() != 0)
		return MANUS_ERROR;

	std::lock_guard<std::mutex> lock(g_gloves_mutex);

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
	std::lock_guard<std::mutex> lock(g_gloves_mutex);

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

int ManusGetState(unsigned int glove, GLOVE_STATE* state, bool blocking)
{
	std::lock_guard<std::mutex> lock(g_gloves_mutex);

	if (glove >= g_gloves.size())
		return MANUS_OUT_OF_RANGE;

	if (!g_gloves[glove]->IsRunning())
		return MANUS_DISCONNECTED;

	if (!state)
		return MANUS_INVALID_ARGUMENT;

	return g_gloves[glove]->GetState(state, blocking) ? MANUS_SUCCESS : MANUS_ERROR;
}

// Taken from the I2CDevice library
// Copyright (c) 2012 Jeff Rowberg
// TODO: Add MIT license information.
int ManusQuaternionToEuler(GLOVE_EULER* v, const GLOVE_QUATERNION* q)
{
	if (!v || !q)
		return MANUS_INVALID_ARGUMENT;

	GLOVE_EULER gravity[1];
	gravity->x = 2 * (q->x*q->z - q->w*q->y);
	gravity->y = 2 * (q->w*q->x + q->y*q->z);
	gravity->z = q->w*q->w - q->x*q->x - q->y*q->y + q->z*q->z;

	// yaw: (about Z axis)
	v->x = atan2(2 * q->x*q->y - 2 * q->w*q->z, 2 * q->w*q->w + 2 * q->x*q->x - 1);
	// pitch: (nose up/down, about Y axis)
	v->y = atan(gravity->x / sqrt(gravity->y*gravity->y + gravity->z*gravity->z));
	// roll: (tilt left/right, about X axis)
	v->z = atan(gravity->y / sqrt(gravity->x*gravity->x + gravity->z*gravity->z));

	return MANUS_SUCCESS;
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
