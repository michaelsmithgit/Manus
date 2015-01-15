#pragma once

#include "Manus.h"

#include <thread>

#define GLOVE_ANGLES 3
#define GLOVE_FINGERS 5

typedef struct
{
	unsigned char id;
	short angles[GLOVE_ANGLES];
	short fingers[GLOVE_FINGERS];
} GLOVE_REPORT;

class Glove
{
private:
	bool m_running;
	unsigned int m_packets;
	GLOVE_REPORT m_report;
	std::thread m_thread;

public:
	Glove(const char* device_path);
	~Glove();

	bool IsRunning() const { return m_running; }
	bool GetState(GLOVE_STATE* state);

private:
	static void DeviceThread(Glove* glove, const char* device_path);
};

