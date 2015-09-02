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
#include "ManusMath.h"

// Sensorfusion constants
#define ACCEL_DIVISOR 16384.0f
#define QUAT_DIVISOR 16384.0f
#define COMPASS_DIVISOR 32.0f
#define FINGER_DIVISOR 255.0f
// magnetometer conversion values
#define FUTPERCOUNT 0.3f; 
#define FCOUNTSPERUT 3.333f;
// accelerometer converion values
#define FGPERCOUNT 0.00006103515f; // 1 / ACCEL_DIVISOR


Glove::Glove(const wchar_t* device_path)
	: m_connected(false)
	, m_service_handle(INVALID_HANDLE_VALUE)
	, m_num_characteristics(0)
	, m_characteristics(nullptr)
	, m_event_handle(INVALID_HANDLE_VALUE)
	, m_value_changed_event(nullptr)
{
	//memset(&m_report, 0, sizeof(m_report));

	size_t len = wcslen(device_path) + 1;
	m_device_path = new wchar_t[len];
	memcpy(m_device_path, device_path, len * sizeof(wchar_t));

	Connect();
}

Glove::~Glove()
{
	Disconnect();
	delete m_device_path;
}

bool Glove::GetState(GLOVE_DATA* data, unsigned int timeout)
{
	// Wait until the thread is done writing a packet
	std::unique_lock<std::mutex> lk(m_report_mutex);

	// Optionally wait until the next package is sent
	if (timeout > 0)
	{
		m_report_block.wait_for(lk, std::chrono::milliseconds(timeout));
		if (!IsConnected())
		{
			lk.unlock();
			return false;
		}
	}
	
	*data = m_data;

	lk.unlock();

	return m_data.PacketNumber > 0;
}

void Glove::Connect()
{
	if (IsConnected())
		Disconnect();

	// Open the device using CreateFile().
	m_service_handle = CreateFile(m_device_path,
		FILE_GENERIC_READ | FILE_GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);

	if (m_service_handle == INVALID_HANDLE_VALUE)
		return;

	// Query the required size for the structures.
	USHORT required_size = 0;
	BluetoothGATTGetCharacteristics(m_service_handle, nullptr, 0, nullptr,
		&required_size, BLUETOOTH_GATT_FLAG_NONE);

	// HRESULT will never be S_OK here, so just check the size.
	if (required_size == 0)
		return;

	// Allocate the characteristic structures.
	m_characteristics = (PBTH_LE_GATT_CHARACTERISTIC)
		malloc(required_size * sizeof(BTH_LE_GATT_CHARACTERISTIC));

	// Get the characteristics offered by this service.
	HRESULT hr = BluetoothGATTGetCharacteristics(m_service_handle, nullptr, required_size, m_characteristics,
		&m_num_characteristics, BLUETOOTH_GATT_FLAG_NONE);

	if (SUCCEEDED(hr))
	{
		// Configure the characteristics.
		for (int i = 0; i < m_num_characteristics; i++)
		{
			if (m_characteristics[i].CharacteristicUuid.Value.ShortUuid == BLE_UUID_MANUS_GLOVE_REPORT)
			{
				ConfigureCharacteristic(&m_characteristics[i], true, false);
			}
			else if (m_characteristics[i].CharacteristicUuid.Value.ShortUuid == BLE_UUID_MANUS_GLOVE_COMPASS)
			{
				ConfigureCharacteristic(&m_characteristics[i], true, false);
			}
			else if (m_characteristics[i].CharacteristicUuid.Value.ShortUuid == BLE_UUID_MANUS_GLOVE_FLAGS)
			{
				ReadCharacteristic(&m_characteristics[i], &m_flags, sizeof(m_flags));
			}
			else if (m_characteristics[i].CharacteristicUuid.Value.ShortUuid == BLE_UUID_MANUS_GLOVE_CALIB)
			{
				ReadCharacteristic(&m_characteristics[i], &m_calib, sizeof(CALIB_REPORT));
			}
		}

		// Allocate the value changed structures.
		m_value_changed_event = (PBLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION)
			malloc(sizeof(PBLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION) + 2 * sizeof(BTH_LE_GATT_CHARACTERISTIC));

		// Register for event callbacks on the characteristics.
		m_value_changed_event->NumCharacteristics = 2;
		memcpy(&m_value_changed_event->Characteristics, m_characteristics, 2 * sizeof(BTH_LE_GATT_CHARACTERISTIC));
		HRESULT hr = BluetoothGATTRegisterEvent(m_service_handle, CharacteristicValueChangedEvent, m_value_changed_event,
			Glove::OnCharacteristicChanged, this, &m_event_handle, BLUETOOTH_GATT_FLAG_NONE);

		m_connected = SUCCEEDED(hr);
	}
}

bool Glove::ReadCharacteristic(PBTH_LE_GATT_CHARACTERISTIC characteristic, void* dest, size_t length)
{
	// Query the required size for the structure.
	USHORT required_size = 0;
	BluetoothGATTGetCharacteristicValue(m_service_handle, characteristic, 0, nullptr,
		&required_size, BLUETOOTH_GATT_FLAG_NONE);

	// HRESULT will never be S_OK here, so just check the size.
	if (required_size == 0)
		return false;

	// Allocate the characteristic value structure.
	PBTH_LE_GATT_CHARACTERISTIC_VALUE value = (PBTH_LE_GATT_CHARACTERISTIC_VALUE)malloc(required_size);

	// Read the characteristic value.
	USHORT actual_size = 0;
	HRESULT hr = BluetoothGATTGetCharacteristicValue(m_service_handle, characteristic, required_size, value,
		&actual_size, BLUETOOTH_GATT_FLAG_NONE);

	// Ensure there is enough room in the buffer.
	if (SUCCEEDED(hr) && length >= value->DataSize)
		memcpy(dest, &value->Data, value->DataSize);

	free(value);
	return SUCCEEDED(hr);
}

bool Glove::WriteCharacteristic(PBTH_LE_GATT_CHARACTERISTIC characteristic, void* src, size_t length)
{
	// Allocate the characteristic value structure.
	PBTH_LE_GATT_CHARACTERISTIC_VALUE value = (PBTH_LE_GATT_CHARACTERISTIC_VALUE)
		malloc(length + sizeof(PBTH_LE_GATT_CHARACTERISTIC_VALUE));

	// Initialize the value structure.
	value->DataSize = length;
	memcpy(value->Data, src, length);

	// Write the characteristic value.
	USHORT actual_size = 0;
	HRESULT hr = BluetoothGATTSetCharacteristicValue(m_service_handle, characteristic, value,
		0, BLUETOOTH_GATT_FLAG_NONE);

	free(value);
	return SUCCEEDED(hr);
}

bool Glove::ConfigureCharacteristic(PBTH_LE_GATT_CHARACTERISTIC characteristic, bool notify, bool indicate)
{
	// Query the required size for the structure.
	USHORT required_size = 0;
	BluetoothGATTGetDescriptors(m_service_handle, characteristic, 0, nullptr,
		&required_size, BLUETOOTH_GATT_FLAG_NONE);

	// HRESULT will never be S_OK here, so just check the size.
	if (required_size == 0)
		return false;

	// Allocate the descriptor structures.
	PBTH_LE_GATT_DESCRIPTOR descriptors = (PBTH_LE_GATT_DESCRIPTOR)
		malloc(required_size * sizeof(BTH_LE_GATT_DESCRIPTOR));

	// Get the descriptors offered by this characteristic.
	USHORT actual_size = 0;
	HRESULT hr = BluetoothGATTGetDescriptors(m_service_handle, characteristic, required_size, descriptors,
		&actual_size, BLUETOOTH_GATT_FLAG_NONE);
	if (SUCCEEDED(hr))
	{
		for (int i = 0; i < actual_size; i++)
		{
			// Look for the client configuration.
			if (descriptors[i].DescriptorType == ClientCharacteristicConfiguration)
			{
				// Configure this characteristic.
				BTH_LE_GATT_DESCRIPTOR_VALUE value;
				memset(&value, 0, sizeof(value));
				value.DescriptorType = ClientCharacteristicConfiguration;
				value.ClientCharacteristicConfiguration.IsSubscribeToNotification = notify;
				value.ClientCharacteristicConfiguration.IsSubscribeToIndication = indicate;

				HRESULT hr = BluetoothGATTSetDescriptorValue(m_service_handle, &descriptors[i], &value, BLUETOOTH_GATT_FLAG_NONE);

				return SUCCEEDED(hr);
			}
		}
	}

	return false;
}

void Glove::Disconnect()
{
	m_connected = false;
	m_report_block.notify_all();

	std::unique_lock<std::mutex> lk(m_report_mutex);

	if (m_event_handle != INVALID_HANDLE_VALUE)
		BluetoothGATTUnregisterEvent(m_event_handle, BLUETOOTH_GATT_FLAG_NONE);
	m_event_handle = INVALID_HANDLE_VALUE;

	if (m_value_changed_event != nullptr)
		free(m_value_changed_event);
	m_value_changed_event = nullptr;

	if (m_service_handle != INVALID_HANDLE_VALUE)
		CloseHandle(m_service_handle);
	m_service_handle = INVALID_HANDLE_VALUE;

	if (m_characteristics != nullptr)
		free(m_characteristics);
	m_characteristics = nullptr;
}

void Glove::OnCharacteristicChanged(BTH_LE_GATT_EVENT_TYPE event_type, void* event_out, void* context)
{
	Glove* glove = (Glove*)context;

	// Normally we would get this parameter from event_out, but it looks like it is an invalid pointer.
	// However it seems the event struct we allocated is being kept up-to-date, so we'll just use that.
	PBLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION changed_event =
		(PBLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION)glove->m_value_changed_event;

	std::lock_guard<std::mutex> lk(glove->m_report_mutex);

	// Read all characteristics we're monitoring.
	for (int i = 0; i < changed_event->NumCharacteristics; i++)
	{
		PBTH_LE_GATT_CHARACTERISTIC characteristic = &changed_event->Characteristics[i];

		if (characteristic->CharacteristicUuid.Value.ShortUuid == BLE_UUID_MANUS_GLOVE_REPORT)
			glove->ReadCharacteristic(characteristic, &glove->m_report, sizeof(GLOVE_REPORT));
		else if (characteristic->CharacteristicUuid.Value.ShortUuid == BLE_UUID_MANUS_GLOVE_COMPASS)
			glove->ReadCharacteristic(characteristic, &glove->m_compass, sizeof(COMPASS_REPORT));
	}

	glove->UpdateState();
	glove->m_report_block.notify_all();
}

void Glove::UpdateState()
{
	// temp data
	AccelSensor myAccel;
	MagSensor myMag;
	fquaternion myQuaternion;

	m_data.PacketNumber++;
	m_data.Handedness = m_flags & GLOVE_FLAGS_HANDEDNESS;

	myAccel.fgPerCount = FGPERCOUNT;

	// normalize acceleration data
	for (int i = 0; i < GLOVE_AXES; i++){
		myAccel.fGp[i] = m_report.accel[i] / ACCEL_DIVISOR;
		myAccel.fGpFast[i] = m_report.accel[i] * FGPERCOUNT;
		myAccel.iGp[i] = m_report.accel[i];
		myAccel.iGpFast[i] = m_report.accel[i];
	}

	// normalize quaternion data
	myQuaternion.q0 = m_report.quat[0] / QUAT_DIVISOR;
	myQuaternion.q1 = m_report.quat[1] / QUAT_DIVISOR;
	myQuaternion.q2 = m_report.quat[2] / QUAT_DIVISOR;
	myQuaternion.q3 = m_report.quat[3] / QUAT_DIVISOR;

	myMag.fCountsPeruT = FCOUNTSPERUT;
	myMag.fuTPerCount = FUTPERCOUNT;

	// normalize magnetometer data
	for (int i = 0; i < GLOVE_AXES; i++){
		myMag.fBp[i] = m_compass.compass[i] / COMPASS_DIVISOR;
		myMag.fBpFast[i] = m_compass.compass[i] * FUTPERCOUNT;
		myMag.iBp[i] = m_compass.compass[i];
		myMag.iBpFast[i] = m_compass.compass[i];
	}

	// normalize finger data
	for (int i = 0; i < GLOVE_FINGERS; i++)
	{
		// account for finger order
		if (m_data.Handedness)
			m_data.Fingers[i] = m_report.fingers[i] / FINGER_DIVISOR;
		else
			m_data.Fingers[i] = m_report.fingers[GLOVE_FINGERS - (i + 1)] / FINGER_DIVISOR;
	}

	// execute the magnetometer and yaw sensor fusion
	fquaternion fused = m_sensorFusion.Fusion_Task(&myAccel, &myMag, &myQuaternion);

	// copy the output of the sensor fusion to m_data
	memcpy(&m_data.Quaternion, &fused, sizeof(GLOVE_QUATERNION));

	// calculate the euler angles
	ManusMath::GetEuler(&m_data.Euler, &m_data.Quaternion);

	// calculate the linear acceleration
	GLOVE_VECTOR gravity, nonLinearAcceleration;

	ManusMath::GetGravity(&gravity, &m_data.Quaternion);
	memcpy(&nonLinearAcceleration, &(myAccel.fGpFast), sizeof(GLOVE_VECTOR));
	ManusMath::GetLinearAcceleration(&m_data.Acceleration, &nonLinearAcceleration, &gravity);

}

uint8_t Glove::GetFlags()
{
	return m_flags;
}

void Glove::SetFlags(uint8_t flags)
{
	m_flags = flags;

	for (int i = 0; i < m_num_characteristics; i++)
	{
		if (m_characteristics[i].CharacteristicUuid.Value.ShortUuid == BLE_UUID_MANUS_GLOVE_FLAGS)
			WriteCharacteristic(&m_characteristics[i], &m_flags, sizeof(m_flags));
	}
}
