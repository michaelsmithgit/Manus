// ManusTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Manus.h"

int _tmain(int argc, _TCHAR* argv[])
{
	ManusInit();

	while (true)
	{
		for (int i = 0; i < ManusGetGloveCount(); i++)
		{
			GLOVE_STATE state = { 0 };
			if (ManusGetState(i, &state, true) == MANUS_SUCCESS)
				printf("glove: %d - %d\n", i, state.PacketNumber);
			else
				printf("glove: %d\n", i);
			
			printf("accel: %f;%f;%f\n", state.data.Acceleration.x, state.data.Acceleration.y, state.data.Acceleration.z);
			printf("quats: %f;%f;%f;%f\n", state.data.Quaternion.x, state.data.Quaternion.y, state.data.Quaternion.z, state.data.Quaternion.w);

			GLOVE_VECTOR euler = { 0 }, gravity;
			ManusGetGravity(&gravity, &state.data.Quaternion);
			printf("gravi: %f;%f;%f\n", gravity.x, gravity.y, gravity.z);
			ManusGetEuler(&euler, &state.data.Quaternion, &gravity);
			printf("euler: %f;%f;%f\n", euler.x, euler.y, euler.z);

			printf("fingers: %f;%f;%f;%f;%f\n", state.data.Fingers[0], state.data.Fingers[1], state.data.Fingers[2], state.data.Fingers[3], state.data.Fingers[4]);
		}
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD());
	}

	ManusExit();

	return 0;
}

