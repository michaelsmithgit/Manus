// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MANUS_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MANUS_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef MANUS_EXPORTS
#define MANUS_API __declspec(dllexport)
#else
#define MANUS_API __declspec(dllimport)
#endif

#define MANUS_ERROR -1
#define MANUS_SUCCESS 0
#define MANUS_INVALID_ARGUMENT 1

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
	MANUS_API int ManusGetState(int glove, GLOVE_STATE* state);

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
