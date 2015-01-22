#pragma once

#include <functional>

class Devices
{
protected:
	std::function<void(const char*)> m_connected;

public:
	virtual ~Devices() {};
	virtual void SetDeviceConnected(std::function<void(const char*)> callback) { m_connected = callback; };
};
