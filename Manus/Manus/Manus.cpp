// Manus.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Manus.h"
#include "Glove.h"
#include "hidapi.h"

#include <vector>

#define MANUS_VENDOR_ID 0x0
#define MANUS_PRODUCT_ID 0x0

std::vector<Glove*> g_gloves;

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
		// The Manus has three interfaces, the third one contains the motion data
		if (current_device->interface_number == 3)
		{
			g_gloves.push_back(new Glove(current_device->path));
		}
		current_device = current_device->next;
	}
	hid_free_enumeration(hid_devices);

	return MANUS_SUCCESS;
}

int ManusExit()
{
	for (Glove* glove : g_gloves)
		delete glove;

	if (hid_exit() != 0)
		return MANUS_ERROR;

	return MANUS_SUCCESS;
}

int ManusGetGloveCount()
{
	return g_gloves.size();
}

int ManusGetState(unsigned int glove, GLOVE_STATE* state)
{
	if (glove >= g_gloves.size())
		return MANUS_OUT_OF_RANGE;

	if (!g_gloves[glove]->IsRunning())
		return MANUS_DISCONNECTED;

	if (!state)
		return MANUS_INVALID_ARGUMENT;

	return g_gloves[glove]->GetState(state) ? MANUS_SUCCESS : MANUS_ERROR;
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
