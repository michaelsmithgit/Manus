#include "stdafx.h"
#include "Glove.h"

Glove::Glove(char* device_path)
	: m_running(false)
	, m_packets(0)
{
	memset(&m_report, 0, sizeof(m_report));
	m_thread = std::thread(DeviceThread, this, device_path);
}

Glove::~Glove()
{
	// Instruct the device thread to stop and
	// wait for it to shut down.
	m_running = false;
	if (m_thread.joinable())
		m_thread.join();
}

bool Glove::GetState(GLOVE_STATE* state)
{
	// TODO: Convert report to state
	return false;
}

void Glove::DeviceThread(Glove* glove, char* device_path)
{
}
