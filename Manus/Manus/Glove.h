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

#pragma once

#include "Manus.h"
#include "SensorFusion.h"

#include <thread>
#include <condition_variable>
#include <mutex>
#include <inttypes.h>

// flag for handedness (0 = left, 1 = right)
#define GLOVE_FLAGS_HANDEDNESS 0x1

#define GLOVE_AXES      3
#define GLOVE_QUATS     4
#define GLOVE_FINGERS   5

#define GLOVE_REPORT_ID     1
#define COMPASS_REPORT_ID   2

#pragma pack(push, 1) // exact fit - no padding
typedef struct
{
	int16_t quat[GLOVE_QUATS];
	int16_t accel[GLOVE_AXES];
	int16_t mag[GLOVE_AXES];
	uint8_t fingers[GLOVE_FINGERS];
} GLOVE_REPORT;

typedef struct
{
	int16_t compass[GLOVE_AXES];
} COMPASS_REPORT;

typedef struct
{
	uint8_t flags;
	uint8_t fingers[GLOVE_FINGERS];
} FLAGS_REPORT;
#pragma pack(pop) //back to whatever the previous packing mode was



class Glove
{
private:
	bool m_running;

	GLOVE_STATE m_state;
	unsigned int m_packets;
	GLOVE_REPORT m_report;
	COMPASS_REPORT m_compass;
	FLAGS_REPORT m_flags;

	char* m_device_path;
	SensorFusion m_sensorFusion;

	std::thread m_thread;
	std::mutex m_report_mutex;
	std::condition_variable m_report_block;

public:
	Glove(const char* device_path);
	~Glove();

	void Connect();
	void Disconnect();
	bool IsRunning() const { return m_running; }
	const char* GetDevicePath() const { return m_device_path; }
	bool GetState(GLOVE_STATE* state, bool blocking);

private:
	static void DeviceThread(Glove* glove);
	static void QuatToEuler(GLOVE_VECTOR* v, const GLOVE_QUATERNION* q);
	void SetState(GLOVE_REPORT *report, COMPASS_REPORT *c_report);
};
