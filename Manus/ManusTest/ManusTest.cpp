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
			
			printf("quats: %f;%f;%f;%f\n", state.data.Quaternion.x, state.data.Quaternion.y, state.data.Quaternion.z, state.data.Quaternion.w);
			printf("euler: %f;%f;%f\n", state.data.Angles.x, state.data.Angles.y, state.data.Angles.z);
		}
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD());
	}

	ManusExit();

	return 0;
}

