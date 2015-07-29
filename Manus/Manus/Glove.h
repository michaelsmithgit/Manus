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

#include <condition_variable>
#include <mutex>
#include <inttypes.h>

// flag for handedness (0 = left, 1 = right)
#define GLOVE_FLAGS_HANDEDNESS  0x1
#define GLOVE_FLAGS_CAL_GYRO    0x2
#define GLOVE_FLAGS_CAL_ACCEL   0x4
#define GLOVE_FLAGS_CAL_FINGERS 0x8

#define GLOVE_AXES      3
#define GLOVE_QUATS     4
#define GLOVE_FINGERS   5

#define BLE_UUID_MANUS_GLOVE_SERVICE    0x0001
#define BLE_UUID_MANUS_GLOVE_REPORT     0x0002
#define BLE_UUID_MANUS_GLOVE_COMPASS    0x0003
#define BLE_UUID_MANUS_GLOVE_FLAGS      0x0004
#define BLE_UUID_MANUS_GLOVE_CALIB      0x0005

// {1bc50001-0200-eca1-e411-20fac04afa8f}
static const GUID GUID_MANUS_GLOVE_SERVICE = { 0x1bc50001, 0x0200, 0xeca1, { 0xe4, 0x11, 0x20, 0xfa, 0xc0, 0x4a, 0xfa, 0x8f } };

#pragma pack(push, 1) // exact fit - no padding
typedef struct
{
	int16_t quat[GLOVE_QUATS];
	int16_t accel[GLOVE_AXES];
	uint8_t fingers[GLOVE_FINGERS];
} GLOVE_REPORT;

typedef struct
{
	int16_t compass[GLOVE_AXES];
} COMPASS_REPORT;

typedef struct
{
	int16_t fingers_base[GLOVE_FINGERS];
	int16_t fingers_range[GLOVE_FINGERS];
} CALIB_REPORT;
#pragma pack(pop) //back to whatever the previous packing mode was

class Glove
{
private:
	bool m_connected;
	uint8_t m_flags;

	GLOVE_STATE m_state;
	unsigned int m_packets;
	GLOVE_REPORT m_report;
	COMPASS_REPORT m_compass;
	CALIB_REPORT m_calib;

	wchar_t* m_device_path;
	SensorFusion m_sensorFusion;

	HANDLE m_service_handle;
	USHORT m_num_characteristics;
	PBTH_LE_GATT_CHARACTERISTIC m_characteristics;
	BLUETOOTH_GATT_EVENT_HANDLE m_event_handle;
	PBLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION m_value_changed_event;

	std::mutex m_report_mutex;
	std::condition_variable m_report_block;

public:
	Glove(const wchar_t* device_path);
	~Glove();

	void Connect();
	void Disconnect();
	bool IsConnected() const { return m_connected; }
	const wchar_t* GetDevicePath() const { return m_device_path; }
	bool GetState(GLOVE_STATE* state, unsigned int timeout);
	uint8_t GetFlags();
	void SetFlags(uint8_t flags);

private:
	static void OnCharacteristicChanged(BTH_LE_GATT_EVENT_TYPE event_type, void* event_out, void* context);
	bool ReadCharacteristic(PBTH_LE_GATT_CHARACTERISTIC characteristic, void* dest, size_t length);
	bool WriteCharacteristic(PBTH_LE_GATT_CHARACTERISTIC characteristic, void* src, size_t length);
	bool ConfigureCharacteristic(PBTH_LE_GATT_CHARACTERISTIC characteristic, bool notify, bool indicate);

	static void QuatToEuler(GLOVE_VECTOR* v, const GLOVE_QUATERNION* q);
	void UpdateState();
};
