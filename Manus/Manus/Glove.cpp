#include "stdafx.h"
#include "Glove.h"
#include "hidapi.h"

#define QUAT_DIVISOR 16384.0f
#define FINGER_DIVISOR 1024.0f

Glove::Glove(const char* device_path)
	: m_running(false)
	, m_packets(0)
{
	memset(&m_report, 0, sizeof(m_report));

	size_t len = strlen(device_path) + 1;
	m_device_path = new char[len];
	memcpy(m_device_path, device_path, len * sizeof(char));

	Connect();
}

Glove::~Glove()
{
	Disconnect();
	delete m_device_path;
}

bool Glove::GetState(GLOVE_STATE* state, bool euler_angles)
{
	state->PacketNumber = m_packets;
	state->data.RightHand = m_report.flags & GLOVE_FLAGS_RIGHTHAND;

	for (int i = 0; i < GLOVE_QUATS; i++)
		((float*)&state->data.Quaternion)[i] = m_report.quat[i] / QUAT_DIVISOR;

	for (int i = 0; i < GLOVE_FINGERS; i++)
		state->data.Fingers[i] = m_report.fingers[i] / FINGER_DIVISOR;

	if (euler_angles)
		QuatToEuler(&state->data.Angles, &state->data.Quaternion);
	else
		memset(&state->data.Angles, 0, sizeof(GLOVE_EULER));

	return m_packets > 0;
}

void Glove::Connect()
{
	Disconnect();
	m_thread = std::thread(DeviceThread, this);
}

void Glove::Disconnect()
{
	// Instruct the device thread to stop and
	// wait for it to shut down.
	m_running = false;
	if (m_thread.joinable())
		m_thread.join();
}

// Taken from the I2CDevic library
// Copyright (c) 2012 Jeff Rowberg
// TODO: Add MIT license information.
void Glove::QuatToEuler(GLOVE_EULER* v, const GLOVE_QUATERNION* q)
{
	if (!v || !q)
		return;

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
}

void Glove::DeviceThread(Glove* glove)
{
	hid_device* device = hid_open_path(glove->m_device_path);
	if (!device)
		return;

	glove->m_running = true;

	// Keep retrieving reports while the SDK is running and the device is connected
	while (glove->m_running && device)
	{
		GLOVE_REPORT report;
		int read = hid_read(device, (unsigned char*)&report, sizeof(report) + 1);

		if (read == -1)
			break;

		// The Manus has three reports, the third one is the raw input report
		// TODO: Check if the bytes read matches the report size
		if (report.id == 3)
		{
			glove->m_report = report;
			glove->m_packets++;
		}
	}

	hid_close(device);

	glove->m_running = false;
}