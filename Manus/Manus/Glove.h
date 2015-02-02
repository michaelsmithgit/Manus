#pragma once

#include "Manus.h"

#include <thread>
#include <condition_variable>
#include <mutex>
#include <inttypes.h>

#define GLOVE_FLAGS_RIGHTHAND 0x1

#define GLOVE_AXES 3
#define GLOVE_QUATS 4
#define GLOVE_FINGERS 5

#pragma pack(push, 1) // exact fit - no padding
typedef struct
{
	uint8_t id;
	uint8_t flags;
	int16_t accel[GLOVE_AXES];
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
	char* m_device_path;

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
};

