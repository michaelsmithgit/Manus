#pragma once

#include <functional>

class Devices
{
protected:
	std::function<void()> m_connected;

public:
	virtual ~Devices() {};
	virtual void SetDeviceConnected(std::function<void()> callback) { m_connected = callback; };
};
