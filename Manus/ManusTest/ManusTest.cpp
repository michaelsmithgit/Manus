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
#include <conio.h>

void clearPrintf(char* text)
{
	printf("|%-60s|\n", text);

}

int _tmain(int argc, _TCHAR* argv[])
{
	ManusInit();

	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);

	clearPrintf("Press 'p' to start reading the gloves");
	clearPrintf("Press 'c' to start the finger calibration procedure");
	clearPrintf("Press 'l' to start logging");

	char in = getch();
	// reset the cursor position
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD());

	if (in == 'c')
	{
		GLOVE_HAND hand;
		clearPrintf("Press 'r' for right hand or 'l' for left hand");
		in = getch();
		if (in == 'l')
			hand = GLOVE_LEFT;
		else
			hand = GLOVE_RIGHT;

		if (ManusCalibrate(hand, false, false, true) != MANUS_SUCCESS)
		{
			clearPrintf("No glove found");
		}
		else
		{
			clearPrintf("Move the flex sensors across their whole range and then press any key");
			getch();
			if (ManusCalibrate(hand, false, false, false) == MANUS_SUCCESS)
				clearPrintf("Calibration finished, press any key to exit");
			else
				clearPrintf("Something went wrong during the calibration");

		}
		getch();
	}
	else if (in == 'p')
	{
		while (true)
		{
			for (int i = 0; i < 2; i++)
			{
				GLOVE_HAND hand = (GLOVE_HAND)i;
				LARGE_INTEGER start, end, elapsed;
				QueryPerformanceCounter(&start);

				GLOVE_DATA data = { 0 };
				GLOVE_SKELETAL skeletal = { 0 };

				if (ManusGetData(hand, &data, 1000) == MANUS_SUCCESS)
				{
					printf("glove: %d - %d %s\n", i, data.PacketNumber, i > 0 ? "Right" : "Left");
					ManusGetSkeletal(hand, &skeletal);
				}
				else
				{
					printf("glove: %d not found \n", i);
					continue;
				}

				QueryPerformanceCounter(&end);
				elapsed.QuadPart = end.QuadPart - start.QuadPart;
				printf("interval: %fms\n", (elapsed.QuadPart * 1000) / (double)freq.QuadPart);


				printf("accel: x: % 1.5f; y: % 1.5f; z: % 1.5f\n", data.Acceleration.x, data.Acceleration.y, data.Acceleration.z);
			
				printf("quats Data: x: % 1.5f; y: % 1.5f; z: % 1.5f; w: % 1.5f \n", data.Quaternion.x, data.Quaternion.y, data.Quaternion.z, data.Quaternion.w);
				printf("quats Skel: x: % 1.5f; y: % 1.5f; z: % 1.5f; w: % 1.5f \n", skeletal.palm.orientation.x , skeletal.palm.orientation.y, skeletal.palm.orientation.z, skeletal.palm.orientation.w);

				printf("euler: x: % 1.5f; y: % 1.5f; z: % 1.5f\n", data.Euler.x * (180.0 / M_PI), data.Euler.y * (180.0 / M_PI), data.Euler.z * (180.0 / M_PI));

				printf("fingers: %f;%f;%f;%f;%f\n", data.Fingers[0], data.Fingers[1], data.Fingers[2], data.Fingers[3], data.Fingers[4]);
			}
			SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD());
		}
	}
	else if (in == 'l')
	{
		FILE* file = fopen("yaw.csv", "a");

		if (file == NULL)
		{
			printf("file error");
		}
		else
		{
			while (_kbhit() == 0)
			{
				GLOVE_DATA data;
				ManusGetData(GLOVE_INDEXED, &data, 1000);
				float yaw = data.Euler.z * (180.0 / M_PI);
				char text[30];
				
				LARGE_INTEGER time;
				QueryPerformanceCounter(&time);


				sprintf(text, "%f,% 1.5f\n", (time.QuadPart * 1000) / (double)freq.QuadPart, yaw);

				printf(text);

				fputs(text, file);
			}
			getch();
			clearPrintf("Logging stopped");
		}

		fclose(file);
		clearPrintf("Press any key to close");
		getch();
	}

	ManusExit();

	return 0;
}

