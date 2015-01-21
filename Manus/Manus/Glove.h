#pragma once

#include "Manus.h"

#include <thread>
#include <inttypes.h>

#define GLOVE_FLAGS_RIGHTHAND 0x1

#define GLOVE_QUATS 4
#define GLOVE_FINGERS 5

#pragma pack(push, 1) // exact fit - no padding
typedef struct
{
	uint8_t id;
	uint8_t flags;
	int16_t quat[GLOVE_QUATS];
	uint16_t fingers[GLOVE_FINGERS];
} GLOVE_REPORT;
#pragma pack(pop) //back to whatever the previous packing mode was

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
	bool GetState(GLOVE_STATE* state, bool euler_angles);

private:
	static void DeviceThread(Glove* glove, const char* device_path);
	static void QuatToEuler(GLOVE_EULER* v, const GLOVE_QUATERNION* q);
};

