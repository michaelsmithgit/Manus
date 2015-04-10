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

#define ACCEL_DIVISOR 16384.0f
#define QUAT_DIVISOR 16384.0f
#define COMPASS_DIVISOR 32.0f
#define FINGER_DIVISOR 255.0f
// magnetometer conversion values
#define FUTPERCOUNT 0.3f; 
#define FCOUNTSPERUT 3.333f;
// accelerometer converion values
#define FGPERCOUNT 0.00006103515; // 2 / (2^15)


Glove::Glove(const char* device_path)
	: m_running(false)
{
	//memset(&m_report, 0, sizeof(m_report));

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
	
	*state = m_state;

	lk.unlock();

	return m_state.PacketNumber > 0;
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

	// Get the flags from the feature report
	unsigned char flags[sizeof(FLAGS_REPORT) + 1];
	flags[0] = 1; // Set feature report ID
	int read = hid_get_feature_report(device, flags, sizeof(flags));

	// If the feature have been read correctly set the flags
	// FIXME: HIDAPI returns the data starting at index 0 instead of index 1
	if (read != -1)
		memcpy(&glove->m_flags, flags, sizeof(FLAGS_REPORT));

	glove->m_running = true;

	// Keep retrieving reports while the SDK is running and the device is connected
	while (glove->m_running && device)
	{
		unsigned char report[sizeof(GLOVE_REPORT) + 1];
		read = hid_read(device, report, sizeof(report));

		if (read == -1)
			break;
		
		// Set the new data report and notify all blocked callers
		// TODO: Check if the bytes read matches the report size
		{
			std::lock_guard<std::mutex> lk(glove->m_report_mutex);

			if (report[0] == GLOVE_REPORT_ID)
				memcpy(&glove->m_report, report + 1, sizeof(GLOVE_REPORT));
			else if (report[0] == COMPASS_REPORT_ID)
				memcpy(&glove->m_compass, report + 1, sizeof(COMPASS_REPORT));

			glove->UpdateState();

			glove->m_report_block.notify_all();
		}
	}

	hid_close(device);

	glove->m_running = false;
	glove->m_report_block.notify_all();
}

void Glove::UpdateState()
{
	// temp data
	AccelSensor myAccel;
	MagSensor myMag;
	fquaternion myQuaternion;
	fquaternion myQuaternionOut;

	m_state.PacketNumber++;
	m_state.data.Handedness = m_flags.flags & GLOVE_FLAGS_HANDEDNESS;

	// normalize acceleration data
	for (int i = 0; i < GLOVE_AXES; i++){
		myAccel.fGpFast[i] = m_report.accel[i] / ACCEL_DIVISOR;
		myAccel.iGp[i] = m_report.accel[i];
		myAccel.iGpFast[i] = m_report.accel[i];
	}

	// normalize quaternion data
	myQuaternion.q0 = m_report.quat[0] / QUAT_DIVISOR;
	myQuaternion.q1 = m_report.quat[1] / QUAT_DIVISOR;
	myQuaternion.q2 = m_report.quat[2] / QUAT_DIVISOR;
	myQuaternion.q3 = m_report.quat[3] / QUAT_DIVISOR;

	// normalize magnetometer data
	for (int i = 0; i < GLOVE_AXES; i++){
		myMag.iBp[i] = m_compass.compass[i];
		myMag.fBp[i] = m_compass.compass[i] / COMPASS_DIVISOR;
		myMag.iBpFast[i] = m_compass.compass[i];
		myMag.fBcFast[i] = m_compass.compass[i] / COMPASS_DIVISOR;
		myMag.fCountsPeruT = FCOUNTSPERUT;
		myMag.fuTPerCount = FUTPERCOUNT;
	}

	// normalize finger data
	for (int i = 0; i < GLOVE_FINGERS; i++)
		m_state.data.Fingers[i] = m_report.fingers[i] / FINGER_DIVISOR;

	// execute the magnetometer and yaw sensor fusion
	m_sensorFusion.fusionTask(&myAccel, &myMag, &myQuaternion, &myQuaternionOut);

	// copy the output of the sensor fusion to m_state
	memcpy(&(m_state.data.Quaternion), &myQuaternionOut, sizeof(GLOVE_QUATERNION));
	memcpy(&(m_state.data.Acceleration), &(myAccel.fGpFast), sizeof(GLOVE_VECTOR));
}
