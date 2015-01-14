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
	bool RightHand;
	float Quaternion[4];
	float Angles[3];
	short Fingers[5];
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
