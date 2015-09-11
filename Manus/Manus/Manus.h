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
	GLOVE_QUATERNION orientation;
	GLOVE_VECTOR position;
} GLOVE_POSE;

typedef struct {
	GLOVE_VECTOR Acceleration;
	GLOVE_VECTOR Euler;
	GLOVE_QUATERNION Quaternion;
	float Fingers[5];
	unsigned int PacketNumber;
} GLOVE_DATA;

typedef struct {
	GLOVE_POSE metacarpal, proximal,
		distal;
} GLOVE_THUMB;

typedef struct {
	GLOVE_POSE metacarpal, proximal,
		intermediate, distal;
} GLOVE_FINGER;

typedef struct {
	GLOVE_POSE palm;
	GLOVE_THUMB thumb;
	GLOVE_FINGER index, middle,
		ring, pinky;
} GLOVE_SKELETAL;

// Left or Right glove enum
typedef enum GLOVE_HAND {
	GLOVE_LEFT = 0,
	GLOVE_RIGHT,
};

/**
* \defgroup Glove Manus Glove
* @{
*/

#define MANUS_ERROR -1
#define MANUS_SUCCESS 0
#define MANUS_INVALID_ARGUMENT 1
#define MANUS_DISCONNECTED 2

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
	*  \param timeout Milliseconds to wait until the glove returns a value.
	*/
	MANUS_API int ManusGetData(GLOVE_HAND hand, GLOVE_DATA* data, unsigned int timeout = 0);

	/*! \brief Get a skeletal model for the given glove state.
	*
	*  The skeletal model gives the orientation and position of each bone
	*  in the hand and fingers. The positions are in millimeters relative to
	*  the position of the hand palm.
	*
	*  Since the thumb has no intermediate phalanx it has a separate structure
	*  in the model.
	* 
	*  \param hand The left or right hand index.
	*  \param state The glove state to derive the skeletal model from.
	*/
	MANUS_API int ManusGetSkeletal(GLOVE_HAND hand, GLOVE_SKELETAL* model);

	/*! \brief Configure the handedness of the glove.
	*
	*  This reconfigures the glove for a different hand.
	*
	*  \warning This function overwrites factory settings on the
	*  glove, it should only be called if the user requested it.
	*
	*  \param hand The left or right hand index.
	*  \param right_hand Set the glove as a right hand.
	*/
	MANUS_API int ManusSetHandedness(GLOVE_HAND hand, bool right_hand);

	/*! \brief Calibrate the IMU on the glove.
	*
	*  This will run a self-test of the IMU and recalibrate it.
	*  The glove should be placed on a stable flat surface during
	*  recalibration.
	*
	*  \warning This function overwrites factory settings on the
	*  glove, it should only be called if the user requested it.
	*
	*  \param hand The left or right hand index.
	*  \param gyro Calibrate the gyroscope.
	*  \param accel Calibrate the accelerometer.
	*  \param fingers Calibrate the fingers.
	*/
	MANUS_API int ManusCalibrate(GLOVE_HAND hand, bool gyro = true, bool accel = true, bool fingers = false);

	/*! \brief Set the ouput power of the vibration motor.
	*
	*  This sets the output power of the vibration motor.
	*
	*  \param glove The glove index.
	*  \param power The power of the vibration motor ranging from 0 to 1 (ex. 0.5 = 50% power).
	*/
	MANUS_API int ManusSetVibration(GLOVE_HAND hand, float power);
}

/**@}*/

#endif
