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

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace ManusMachina
{
#pragma warning disable 0649 // Disable 'field never assigned' warning
    [StructLayout(LayoutKind.Sequential)]
    public struct GLOVE_QUATERNION
    {
        public float w, x, y, z;

        public GLOVE_QUATERNION(float w, float x, float y, float z)
        {
            this.w = w;
            this.x = x;
            this.y = y;
            this.z = z;
        }

        public static GLOVE_QUATERNION operator *(GLOVE_QUATERNION a, GLOVE_QUATERNION b)
        {
            return new GLOVE_QUATERNION(
                a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
                a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
                a.w * b.y - a.x * b.y + a.y * b.w + a.z * b.x,
                a.w * b.z + a.x * b.z - a.y * b.x + a.z * b.w);
        }

        public float this[int index]
        {
            get
            {
                switch (index)
                {
                    case 0: return this.w;
                    case 1: return this.x;
                    case 2: return this.y;
                    case 3: return this.z;
                    default: throw new InvalidOperationException();
                }
            }
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct GLOVE_VECTOR
    {
        public float x, y, z;

        public GLOVE_VECTOR(float x, float y, float z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }

        public GLOVE_VECTOR ToDegrees()
        {
            return new GLOVE_VECTOR((float)(x * 180.0 / Math.PI), (float)(y * 180.0 / Math.PI), (float)(z * 180.0 / Math.PI));
        }

        public float[] ToArray()
        {
            return new float[3] { this.x, this.y, this.z };
        }

        public float this[int index]
        {
            get
            {
                switch (index)
                {
                    case 0: return this.x;
                    case 1: return this.y;
                    case 2: return this.z;
                    default: throw new InvalidOperationException();
                }
            }

            set
            {
                switch (index)
                {
                    case 0: this.x = value; return;
                    case 1: this.y = value; return;
                    case 2: this.z = value; return;
                    default: throw new InvalidOperationException();
                }
            }
        }


        public static GLOVE_VECTOR operator +(GLOVE_VECTOR a, GLOVE_VECTOR b)
        {
            return new GLOVE_VECTOR(a.x + b.x, a.y + b.y, a.z + b.z);
        }
        public static GLOVE_VECTOR operator -(GLOVE_VECTOR a, GLOVE_VECTOR b)
        {
            return new GLOVE_VECTOR(a.x - b.x, a.y - b.y, a.z - b.z);
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct GLOVE_DATA
    {
        public bool RightHand;
        public GLOVE_VECTOR Acceleration;
        public GLOVE_QUATERNION Quaternion;

        [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 5)]
        public float[] Fingers;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct GLOVE_STATE
    {
        public uint PacketNumber;
        public GLOVE_DATA data;
    }
#pragma warning restore 0649

    public class Manus
    {
        public const int ERROR = -1;
        public const int SUCCESS = 0;
        public const int INVALID_ARGUMENT = 1;
        public const int OUT_OF_RANGE = 2;
        public const int DISCONNECTED = 3;


        /*! \brief Initialize the Manus SDK.
        *
        *  Must be called before any other function
        *  in the SDK.
        */
        [DllImport("Manus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int ManusInit();

        /*! \brief Shutdown the Manus SDK.
        *
        *  Must be called when the SDK is no longer
        *  needed.
        */
        [DllImport("Manus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int ManusExit();

        /*! \brief Get the number of gloves.
        *
        *  Get the maximum index that can be queried
        *  for the glove state.
        */
        [DllImport("Manus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int ManusGetGloveCount();

        /*! \brief Get the state of a glove.
        *
        *  \param glove The glove index.
        *  \param state Output variable to receive the state.
        *  \param timeout Milliseconds to wait until the glove returns a value.
        */
        [DllImport("Manus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int ManusGetState(uint glove, out GLOVE_STATE state, uint timeout = 0);

        /*! \brief Convert a Quaternion to Euler angles.
        *
        *  Returns the Quaternion as Yaw, Pitch and Roll angles
        *  relative to the Earth's gravity.
        *
        *  \param euler Output variable to receive the Euler angles.
        *  \param quaternion The quaternion to convert.
        */
        [DllImport("Manus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int ManusGetEuler(out GLOVE_VECTOR euler, ref GLOVE_QUATERNION quaternion);

        /*! \brief Remove gravity from acceleration vector.
        *
        *  Returns the Acceleration as a vector independent from
        *  the Earth's gravity.
        *
        *  \param linear Output vector to receive the linear acceleration.
        *  \param acceleration The acceleration vector to convert.
        */
        [DllImport("Manus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int ManusGetLinearAcceleration(out GLOVE_VECTOR linear, ref GLOVE_VECTOR acceleration, ref GLOVE_VECTOR gravity);

        /*! \brief Return gravity vector from the Quaternion.
        *
        *  Returns an estimation of the Earth's gravity vector.
        *
        *  \param gravity Output vector to receive the gravity vector.
        *  \param quaternion The quaternion to base the gravity vector on.
        */
        [DllImport("Manus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int ManusGetGravity(out GLOVE_VECTOR gravity, ref GLOVE_QUATERNION quaternion);

        /*! \brief Configure the handedness of the glove.
        *
        *  This reconfigures the glove for a different hand.
        *
        *  \warning This function overwrites factory settings on the
        *  glove, it should only be called if the user requested it.
        *
        *  \param glove The glove index.
        *  \param right_hand Set the glove as a right hand.
        */
        [DllImport("Manus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int ManusSetHandedness(uint glove, bool right_hand);

        /*! \brief Calibrate the IMU on the glove.
        *
        *  This will run a self-test of the IMU and recalibrate it.
        *  The glove should be placed on a stable flat surface during
        *  recalibration.
        *
        *  \warning This function overwrites factory settings on the
        *  glove, it should only be called if the user requested it.
        *
        *  \param glove The glove index.
        *  \param gyro Calibrate the gyroscope.
        *  \param accel Calibrate the accelerometer.
        *  \param fingers Calibrate the finger flex sensors.
        */
        [DllImport("Manus.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int ManusCalibrate(uint glove, bool gyro = true, bool accel = true, bool fingers = true);
    }
}
