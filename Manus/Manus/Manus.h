#ifndef _MANUS_H
#define _MANUS_H

#ifdef MANUS_EXPORTS
#define MANUS_API __declspec(dllexport)
#else
#define MANUS_API __declspec(dllimport)
#endif

#define MANUS_ERROR -1
#define MANUS_SUCCESS 0
#define MANUS_INVALID_ARGUMENT 1
#define MANUS_OUT_OF_RANGE 2
#define MANUS_DISCONNECTED 3

typedef struct {
	float w, x, y, z;
} GLOVE_QUATERNION;

typedef struct {
	float x, y, z;
} GLOVE_EULER;

typedef struct {
	bool RightHand;
	GLOVE_QUATERNION Quaternion;
	float Fingers[5];
} GLOVE_DATA;

typedef struct {
	unsigned int PacketNumber;
	GLOVE_DATA data;
} GLOVE_STATE;

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
	*/
	MANUS_API int ManusGetState(unsigned int glove, GLOVE_STATE* state);

	/*! \brief Convert a Quaternion to Euler angles.
	*
	*  Returns the Quaternion as Yaw, Pitch and Roll angles.
	*
	*  \param euler Output variable to receive the Euler angles.
	*  \param quaternion The quaternion to convert.
	*/
	MANUS_API int ManusQuaternionToEuler(GLOVE_EULER* euler, const GLOVE_QUATERNION* quaternion);

	/*! \brief Enable gamepad emulation.
	*
	*  Allows the SDK to convert glove data to gamepad
	*  input.
	*/
	MANUS_API int ManusEnableGamepad(bool enabled);

	/*! \brief Enable mouse emulation.
	*
	*  Allows the SDK to convert glove data to mouse
	*  input.
	*/
	MANUS_API int ManusEnableKeyboard(bool enabled);

	/*! \brief Enable keyboard emulation.
	*
	*  Allows the SDK to convert glove data to keyboard
	*  input.
	*/
	MANUS_API int ManusEnableMouse(bool enabled);
}

#endif
