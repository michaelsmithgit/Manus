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

#define _USE_MATH_DEFINES
#include <math.h>

int _tmain(int argc, _TCHAR* argv[])
{
	ManusInit();

	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);

	while (true)
	{
		for (int i = 0; i < 2; i++)
		{
			GLOVE_HAND hand = (GLOVE_HAND)i;
			LARGE_INTEGER start, end, elapsed;
			QueryPerformanceCounter(&start);

			GLOVE_DATA data = { 0 };
			if (ManusGetData(hand, &data, 1000) == MANUS_SUCCESS){
				printf("glove: %d - %d %d %s\n", i, data.PacketNumber, "Right");
			}else{
				printf("glove: %d not found \n", i);
				continue;
			}

			QueryPerformanceCounter(&end);
			elapsed.QuadPart = end.QuadPart - start.QuadPart;
			printf("interval: %fms\n", (elapsed.QuadPart * 1000) / (double)freq.QuadPart);


			printf("accel: x: % 1.5f; y: % 1.5f; z: % 1.5f\n", data.Acceleration.x, data.Acceleration.y, data.Acceleration.z);
			
			printf("quats: x: % 1.5f; y: % 1.5f; z: % 1.5f; w: % 1.5f \n", data.Quaternion.x, data.Quaternion.y, data.Quaternion.z, data.Quaternion.w);

			printf("euler: x: % 1.5f; y: % 1.5f; z: % 1.5f\n", data.Euler.x * (180.0 / M_PI), data.Euler.y * (180.0 / M_PI), data.Euler.z * (180.0 / M_PI));

			printf("fingers: %f;%f;%f;%f;%f\n", data.Fingers[0], data.Fingers[1], data.Fingers[2], data.Fingers[3], data.Fingers[4]);
		}
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD());
	}

	ManusExit();

	return 0;
}

