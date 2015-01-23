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

bool Glove::GetState(GLOVE_STATE* state)
{
	state->PacketNumber = m_packets;
	state->data.RightHand = m_report.flags & GLOVE_FLAGS_RIGHTHAND;

	for (int i = 0; i < GLOVE_QUATS; i++)
		((float*)&state->data.Quaternion)[i] = m_report.quat[i] / QUAT_DIVISOR;

	for (int i = 0; i < GLOVE_FINGERS; i++)
		state->data.Fingers[i] = m_report.fingers[i] / FINGER_DIVISOR;

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
