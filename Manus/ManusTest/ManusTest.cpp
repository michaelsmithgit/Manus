// ManusTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Manus.h"

#include <stdio.h>

int _tmain(int argc, _TCHAR* argv[])
{
	ManusInit();

	while (true)
	{
		for (int i = 0; i < ManusGetGloveCount(); i++)
		{
			GLOVE_STATE state;
			ManusGetState(i, &state, true);
			printf("glove: %d - %d\n", i, state.PacketNumber);
			printf("quats: %f;%f;%f;%f\n", state.data.Quaternion.x, state.data.Quaternion.y, state.data.Quaternion.z, state.data.Quaternion.w);
			printf("euler: %f;%f;%f\n", state.data.Angles.x, state.data.Angles.y, state.data.Angles.z);
		}
	}

	ManusExit();

	return 0;
}

