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

#ifndef _MANUS_H
#define _MANUS_H

#ifdef MANUS_EXPORTS
#define MANUS_API __declspec(dllexport)
#else
#define MANUS_API __declspec(dllimport)
#endif

typedef struct {
	float w, x, y, z;
} GLOVE_QUATERNION;

typedef struct {
	float x, y, z;
} GLOVE_VECTOR;

typedef struct {
	bool Handedness;
	GLOVE_VECTOR Acceleration;
	GLOVE_QUATERNION Quaternion;
	GLOVE_VECTOR Magnetometer;
} GLOVE_DATA;

typedef struct {
	unsigned int PacketNumber;
	GLOVE_DATA data;
} GLOVE_STATE;

/**
* \defgroup Glove Manus Glove
* @{
*/

#define MANUS_ERROR -1
#define MANUS_SUCCESS 0
#define MANUS_INVALID_ARGUMENT 1
#define MANUS_OUT_OF_RANGE 2
#define MANUS_DISCONNECTED 3

extern "C" {
	/*! \brief Initialize the Manus SDK.
	*
	*  Must be called before any other function
	*  in the SDK.
	*/
	MANUS_API int ManusInit();

	/*! \brief Shutdown the Manus SDK.
	*
	*  Must be called when the SDK is no longer
	*  needed.
	*/
	MANUS_API int ManusExit();

	/*! \brief Get the number of gloves.
	*
	*  Get the maximum index that can be queried
	*  for the glove state.
	*/
	MANUS_API int ManusGetGloveCount();

	/*! \brief Get the state of a glove.
	*
	*  \param glove The glove index.
	*  \param state Output variable to receive the state.
	*  \param blocking Wait until the glove returns a value.
	*/
	MANUS_API int ManusGetState(unsigned int glove, GLOVE_STATE* state, bool blocking = false);

	/*! \brief Convert a Quaternion to Euler angles.
	*
	*  Returns the Quaternion as Yaw, Pitch and Roll angles
	*  relative to the Earth's gravity.
	*
	*  \param euler Output variable to receive the Euler angles.
	*  \param quaternion The quaternion to convert.
	*/
	MANUS_API int ManusGetEuler(GLOVE_VECTOR* euler, const GLOVE_QUATERNION* quaternion);

	/*! \brief Remove gravity from acceleration vector.
	*
	*  Returns the Acceleration as a vector independent from
	*  the Earth's gravity.
	*
	*  \param linear Output vector to receive the linear acceleration.
	*  \param acceleation The acceleration vector to convert.
	*/
	MANUS_API int ManusGetLinearAcceleration(GLOVE_VECTOR* linear, const GLOVE_VECTOR* acceleration, const GLOVE_VECTOR* gravity);

	/*! \brief Return gravity vector from the Quaternion.
	*
	*  Returns an estimation of the Earth's gravity vector.
	*
	*  \param gravity Output vector to receive the gravity vector.
	*  \param quaternion The quaternion to base the gravity vector on.
	*/
	MANUS_API int ManusGetGravity(GLOVE_VECTOR* gravity, const GLOVE_QUATERNION* quaternion);
}

/**@}*/

#endif
