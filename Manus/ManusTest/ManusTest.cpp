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
			if (ManusGetState(i, &state) == MANUS_SUCCESS)
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

