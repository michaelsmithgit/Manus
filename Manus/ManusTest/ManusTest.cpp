// ManusTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Manus.h"

#include <stdio.h>

int _tmain(int argc, _TCHAR* argv[])
{
	ManusInit();

	int last_packet = 0;
	while (true)
	{
		for (int i = 0; i < ManusGetGloveCount(); i++)
		{
			GLOVE_STATE state;
			ManusGetState(i, &state);
			if (state.PacketNumber != last_packet)
			{
				last_packet = state.PacketNumber;
				printf("%f;%f;%f;%f\n", state.data.Quaternion[0], state.data.Quaternion[1], state.data.Quaternion[2], state.data.Quaternion[3]);
			}
		}
	}

	ManusExit();

	return 0;
}

