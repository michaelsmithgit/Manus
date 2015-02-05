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
#include "hidapi.h"

#define ACCEL_DIVISOR 8192.0f
#define QUAT_DIVISOR 16384.0f
#define FINGER_DIVISOR 300.0f

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

bool Glove::GetState(GLOVE_STATE* state, bool blocking)
{
	// Wait until the thread is done writing a packet
	std::unique_lock<std::mutex> lk(m_report_mutex);

	// Optionally wait until the next package is sent
	if (blocking)
	{
		m_report_block.wait(lk);
		if (!m_running)
		{
			lk.unlock();
			return false;
		}
	}

	state->PacketNumber = m_packets;
	state->data.RightHand = m_report.flags & GLOVE_FLAGS_RIGHTHAND;

	for (int i = 0; i < GLOVE_AXES; i++)
		((float*)&state->data.Acceleration)[i] = m_report.accel[i] / ACCEL_DIVISOR;

	for (int i = 0; i < GLOVE_QUATS; i++)
		((float*)&state->data.Quaternion)[i] = m_report.quat[i] / QUAT_DIVISOR;

	for (int i = 0; i < GLOVE_FINGERS; i++)
		state->data.Fingers[i] = m_report.fingers[i] / FINGER_DIVISOR;

	lk.unlock();

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
		int read = hid_read(device, (unsigned char*)&report, sizeof(report));

		if (read == -1)
			break;

		// Set the new data report and notify all blocked callers
		// TODO: Check if the bytes read matches the report size
		{
			std::lock_guard<std::mutex> lk(glove->m_report_mutex);

			glove->m_report = report;
			glove->m_packets++;
			glove->m_report_block.notify_all();
		}
	}

	hid_close(device);

	glove->m_running = false;
	glove->m_report_block.notify_all();
}
