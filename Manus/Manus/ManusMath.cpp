#include "stdafx.h"
#include "Glove.h"
#include "ManusMath.h"


ManusMath::ManusMath()
{
}

ManusMath::~ManusMath()
{
}

int ManusMath::GetEuler(GLOVE_VECTOR* v, const GLOVE_QUATERNION* q)
{
	if (!v || !q)
		return MANUS_INVALID_ARGUMENT;

	// roll: (tilt left/right, about X axis)
	v->x = atan2(2 * (q->w * q->x + q->y * q->z), 1 - 2 * (q->x * q->x + q->y * q->y));
	// pitch: (nose up/down, about Y axis)
	v->y = asin(2 * (q->w * q->y - q->z * q->x));
	// yaw: (about Z axis)
	v->z = atan2(2 * (q->w * q->z + q->x * q->y), 1 - 2 * (q->y * q->y + q->z * q->z));

	return MANUS_SUCCESS;
}

// Taken from the I2CDevice library
// Copyright (c) 2012 Jeff Rowberg
// TODO: Add MIT license information.
int ManusMath::GetLinearAcceleration(GLOVE_VECTOR* v, const GLOVE_VECTOR* vRaw, const GLOVE_VECTOR* gravity)
{
	if (!v || !vRaw || !gravity)
		return MANUS_INVALID_ARGUMENT;

	v->x = vRaw->x - gravity->x;
	v->y = vRaw->y - gravity->y;
	v->z = vRaw->z - gravity->z;

	return MANUS_SUCCESS;
}

// Taken from the I2CDevice library
// Copyright (c) 2012 Jeff Rowberg
// TODO: Add MIT license information.
int ManusMath::GetGravity(GLOVE_VECTOR* gravity, const GLOVE_QUATERNION* q)
{
	if (!gravity || !q)
		return MANUS_INVALID_ARGUMENT;

	gravity->x = 2 * (q->x*q->z - q->w*q->y);
	gravity->y = 2 * (q->w*q->x + q->y*q->z);
	gravity->z = q->w*q->w - q->x*q->x - q->y*q->y + q->z*q->z;

	return MANUS_SUCCESS;
}