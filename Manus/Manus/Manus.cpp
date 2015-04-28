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
#include "Manus.h"
#include "Glove.h"
#include "Devices.h"
#include "hidapi.h"

#ifdef _WIN32
#include "WinDevices.h"
#endif

#include <vector>
#include <mutex>

// TODO: Acquire Manus VID/PID
#define MANUS_VENDOR_ID         0x0
#define MANUS_PRODUCT_ID        0x0
#define MANUS_GLOVE_PAGE	    0x03
#define MANUS_GLOVE_USAGE       0x04

std::vector<Glove*> g_gloves;
std::mutex g_gloves_mutex;

Devices* g_devices;

int GetGlove(unsigned int glove, Glove** elem)
{
	std::lock_guard<std::mutex> lock(g_gloves_mutex);

	if (glove >= g_gloves.size())
		return MANUS_OUT_OF_RANGE;

	if (!g_gloves[glove]->IsRunning())
		return MANUS_DISCONNECTED;

	*elem = g_gloves[glove];

	return MANUS_SUCCESS;
}

void DeviceConnected(const char* device_path)
{
	std::lock_guard<std::mutex> lock(g_gloves_mutex);

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
		// We're only interested in devices that identify themselves as VR Gloves
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
	return (int)g_gloves.size();
}

int ManusGetState(unsigned int glove, GLOVE_STATE* state, unsigned int blocking)
{
	// Get the glove from the list
	Glove* elem;
	int ret = GetGlove(glove, &elem);
	if (ret != MANUS_SUCCESS)
		return ret;

	if (!state)
		return MANUS_INVALID_ARGUMENT;

	return elem->GetState(state, blocking) ? MANUS_SUCCESS : MANUS_ERROR;
}

int ManusGetEuler(GLOVE_VECTOR* v, const GLOVE_QUATERNION* q)
{
	if (!v || !q)
		return MANUS_INVALID_ARGUMENT;

	// roll: (tilt left/right, about X axis)
	v->x = atan2(2 * (q->w * q->x + q->y * q->z), 1 - 2 * (q->x * q->x + q->y * q->y));
	// pitch: (nose up/down, about Y axis)
	v->y = asin(2 * (q->w * q->y - q->z * q->x));
	// yaw: (about Z axis)
	v->z = atan2(2 * (q->w * q->z + q->x * q->y), 1 - 2 * (q->y * q->y + q->z * q->z));

	return MANUS_SUCCESS;
}

// Taken from the I2CDevice library
// Copyright (c) 2012 Jeff Rowberg
// TODO: Add MIT license information.
int ManusGetLinearAcceleration(GLOVE_VECTOR* v, const GLOVE_VECTOR* vRaw, const GLOVE_VECTOR* gravity)
{
	if (!v || !vRaw || !gravity)
		return MANUS_INVALID_ARGUMENT;

	v->x = vRaw->x - gravity->x;
	v->y = vRaw->y - gravity->y;
	v->z = vRaw->z - gravity->z;

	return MANUS_SUCCESS;
}

// Taken from the I2CDevice library
// Copyright (c) 2012 Jeff Rowberg
// TODO: Add MIT license information.
int ManusGetGravity(GLOVE_VECTOR* gravity, const GLOVE_QUATERNION* q)
{
	if (!gravity || !q)
		return MANUS_INVALID_ARGUMENT;

	gravity->x = 2 * (q->x*q->z - q->w*q->y);
	gravity->y = 2 * (q->w*q->x + q->y*q->z);
	gravity->z = q->w*q->w - q->x*q->x - q->y*q->y + q->z*q->z;

	return MANUS_SUCCESS;
}

int ManusSetHandedness(unsigned int glove, bool right_hand)
{
	// Get the glove from the list
	Glove* elem;
	int ret = GetGlove(glove, &elem);
	if (ret != MANUS_SUCCESS)
		return ret;

	// Set the flags
	uint8_t flags = elem->GetFlags();
	if (right_hand)
		flags |= GLOVE_FLAGS_HANDEDNESS;
	else
		flags &= ~GLOVE_FLAGS_HANDEDNESS;
	elem->SetFlags(flags);

	return MANUS_SUCCESS;
}

int ManusCalibrate(unsigned int glove, bool gyro, bool accel, bool fingers)
{
	// Get the glove from the list
	Glove* elem;
	int ret = GetGlove(glove, &elem);
	if (ret != MANUS_SUCCESS)
		return ret;

	// Set the flags
	uint8_t flags = elem->GetFlags();
	if (gyro)
		flags |= GLOVE_FLAGS_CAL_GYRO;
	if (accel)
		flags |= GLOVE_FLAGS_CAL_ACCEL;
	if (fingers)
		flags |= GLOVE_FLAGS_CAL_FINGERS;
	elem->SetFlags(flags);

	return MANUS_SUCCESS;
}
