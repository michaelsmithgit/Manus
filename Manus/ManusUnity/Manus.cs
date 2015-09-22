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
using System.Runtime.InteropServices;
using UnityEngine;
using System.Threading;



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
    public struct GLOVE_POSE
    {
        public GLOVE_QUATERNION orientation;
        public GLOVE_VECTOR position;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct GLOVE_DATA
    {
        /* old GLOVE_DATA definition, mismatch with .h file
        public bool RightHand;
        public GLOVE_VECTOR Acceleration;
        public GLOVE_QUATERNION Quaternion;

        [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 5)]
        public float[] Fingers;
        */

        public GLOVE_VECTOR Acceleration;
        public GLOVE_VECTOR Euler;
        public GLOVE_QUATERNION Quaternion;
        [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 5)]
        public float[] Fingers;
        public uint PacketNumber;

    }

    [StructLayout(LayoutKind.Sequential)]
    public struct GLOVE_THUMB
    {
        public GLOVE_POSE metacarpal, proximal,
            distal;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct GLOVE_FINGER
    {
        public GLOVE_POSE metacarpal, proximal,
            intermediate, distal;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct GLOVE_SKELETAL
    {
        public GLOVE_POSE palm;
        public GLOVE_THUMB thumb;
        public GLOVE_FINGER index, middle,
            ring, pinky;
    }

#pragma warning restore 0649

    public enum GLOVE_HAND
    {
        GLOVE_LEFT = 0,
        GLOVE_RIGHT,
    };


    /*!
    *   \brief Thumb class
    * 
    *   Thumb class, to be used in Unity project
    */
    [System.Serializable]
    public class Thumb
    {
        public Transform Metacarpal, Proximal, Distal;
    }


    /*!
    *   \brief Finger class
    * 
    *   Finger class, to be used in Unity project
    */
    [System.Serializable]
    public class Finger
    {
        public Transform Metacarpal, Proximal, Intermediate, Distal;
    }

    /*!
    *   \brief Skeletal class
    * 
    *   Skeletal class, to be used in Unity project
    */
    [System.Serializable]
    public class Skeletal {
        public Transform Palm;
        public Thumb Thumb = new Thumb();
        public Finger Index = new Finger(), Middle = new Finger(), Ring = new Finger(), Pinky = new Finger();
    }


    /*!
    *   \brief Glove class
    *
    */
    [System.Serializable]
    public class Glove {
        public const int ERROR = -1;
        public const int SUCCESS = 0;
        public const int INVALID_ARGUMENT = 1;
        public const int OUT_OF_RANGE = 2;
        public const int DISCONNECTED = 3;

        private GLOVE_HAND hand;                    /*!< Is this a right or left hand glove */

        private GLOVE_DATA data = new GLOVE_DATA(); /*!< Glove Data Struct */
        private Thread getDataThread;               /*!< Glove Data Thread */
        private Mutex  getDataMutex;                /*!< Glove Data Mutex  */

        /*! \brief Glove Data Thread Function
        *
        *  This function gets the data from the (C++) library,
        *  ensuring most recent data is available in the class.
        */
        private void getDataThreadFunc(){
            while (true) { // note: implement other stopping mechanism then abort() 
                getDataMutex.WaitOne();
                ManusGetData(hand, ref data, 1000); //note: should the timeout be configurable?
                getDataMutex.ReleaseMutex();
            }
        }

        /*! \brief Initialize the Manus SDK.
        *
        *  Must be called before any other function
        *  in the SDK.
        */
        [DllImport("Manus.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int ManusInit();

        /*! \brief Shutdown the Manus SDK.
        *
        *  Must be called when the SDK is no longer
        *  needed.
        */
        [DllImport("Manus.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int ManusExit();


        /*! \brief Get the state of a glove.
        *
        *  \param hand The left or right hand index.
        *  \param state Output variable to receive the data.
        *  \param timeout Milliseconds to wait until the glove returns a value.
        */
        [DllImport("Manus.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int ManusGetData(GLOVE_HAND hand, ref GLOVE_DATA data, uint timeout = 0);


        /*! \brief Get a skeletal model for the given glove state.
        *
        *  The skeletal model gives the orientation and position of each bone
        *  in the hand and fingers. The positions are in millimeters relative to
        *  the position of the hand palm.
        *
        *  Since the thumb has no intermediate phalanx it has a separate structure
        *  in the model.
        * 
        *  \param hand The left or right hand index.
        *  \param model The glove skeletal model.
        */
        [DllImport("Manus.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int ManusGetSkeletal(GLOVE_HAND hand, ref GLOVE_SKELETAL model, uint timeout=1000);

        /*! \brief Configure the handedness of the glove.
        *
        *  This reconfigures the glove for a different hand.
        *
        *  \warning This function overwrites factory settings on the
        *  glove, it should only be called if the user requested it.
        *
        *  \param hand The left or right hand index.
        *  \param right_hand Set the glove as a right hand.
        */
        [DllImport("Manus.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int ManusSetHandedness(GLOVE_HAND hand, bool right_hand);

        /*! \brief Calibrate the IMU on the glove.
        *
        *  This will run a self-test of the IMU and recalibrate it.
        *  The glove should be placed on a stable flat surface during
        *  recalibration.
        *
        *  \warning This function overwrites factory settings on the
        *  glove, it should only be called if the user requested it.
        *
        *  \param hand The left or right hand index.
        *  \param gyro Calibrate the gyroscope.
        *  \param accel Calibrate the accelerometer.
        *  \param fingers Calibrate the fingers.
        */
        [DllImport("Manus.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int ManusCalibrate(GLOVE_HAND hand, bool gyro = true, bool accel = true, bool fingers = false);

        /*! \brief Set the ouput power of the vibration motor.
        *
        *  This sets the output power of the vibration motor.
        *
        *  \param glove The glove index.
        *  \param power The power of the vibration motor ranging from 0 to 1 (ex. 0.5 = 50% power).
        */
        [DllImport("Manus.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int ManusSetVibration(GLOVE_HAND hand, float power);


        /*! \brief Convert Quaternion from Manus to Unity
        *
        * This converts a Quaternion from Manus to Unity format
        *
        * \param q Quaternion in Manus format
        * \return Quaternion in Unity format
        */
        private static Quaternion ManusToUnity(GLOVE_QUATERNION q) {
            return new Quaternion(-q.x/100.0f, q.y/100.0f, q.z/100.0f, -q.w/100.0f);
        }

        /*! \brief Convert Vector from Manus to Unity
        *
        * This converts a Vector from Manus to Unity format
        *
        * \param q Vector in Manus format
        * \return Vector in Unity format
        */
        private static Vector3 ManusToUnity(GLOVE_VECTOR v) {
            return new Vector3( -v.x/100.0f, v.y/100.0f, v.z/100.0f);
        }


        /*! \brief Convert Manus Pose to Unity Transform
        *
        * \param unity Unity Transform Object, which will store the result
        * \param manus GLOVE_POSE Manus Thumb Struct to be converted
        */
        private void ManusToUnity(ref Transform unity, GLOVE_POSE manus) {
            unity.position = ManusToUnity(manus.position);
            unity.rotation = ManusToUnity(manus.orientation);
        }


        /*! \brief Convert Manus Finger to Unity Finger
        *
        * \param unityFinger Unity Finger Object, which will store the result
        * \param manusFinger GLOVE_FINGER Manus Finger Struct to be converted
        */
        private void ManusToUnity(ref Finger unityFinger, GLOVE_FINGER manusFinger) {
            ManusToUnity(ref unityFinger.Distal, manusFinger.distal);
            ManusToUnity(ref unityFinger.Intermediate, manusFinger.intermediate);
            ManusToUnity(ref unityFinger.Metacarpal, manusFinger.metacarpal);
            ManusToUnity(ref unityFinger.Proximal, manusFinger.proximal);
        }

        /*! \brief Convert Manus Thumb to Unity Thumb
        *
        * \param unityThumb Unity Thumb Object, which will store the result
        * \param manus GLOVE_THUMB Manus Thumb Struct to be converted
        */
        private void ManusToUnity(ref Thumb unityThumb, GLOVE_THUMB manusThumb) {
            ManusToUnity(ref unityThumb.Distal, manusThumb.distal);
            ManusToUnity(ref unityThumb.Metacarpal, manusThumb.metacarpal);
            ManusToUnity(ref unityThumb.Proximal, manusThumb.proximal);
        }

        /*! \brief Acceleration in Unity format
        */
        public Vector3 Acceleration { get {
                getDataMutex.WaitOne();
                Vector3 result = ManusToUnity(data.Acceleration);
                getDataMutex.ReleaseMutex();
                return result;
            }
        }


        /*! \brief Euler vector in Unity format
        */
        public Vector3 Euler { get {
                getDataMutex.WaitOne();
                Vector3 result = ManusToUnity(data.Euler);
                getDataMutex.ReleaseMutex();
                return result;
            }
        }

        /*! \brief Fingers array
         */
        public float[] Fingers { get {
                getDataMutex.WaitOne();
                float[] result = data.Fingers;
                getDataMutex.ReleaseMutex();
                return result;
            }
        }

        /*! \brief Quaternion in Unity format
        *
        * Returns the current Quaternion in Unity format.
        */
        public Quaternion Quaternion { get {
                getDataMutex.WaitOne();
                Quaternion result = ManusToUnity(data.Quaternion);
                getDataMutex.ReleaseMutex();
                return result;
            }
        }

        /*! Get whether is is a right or left hand.
        */
        public GLOVE_HAND GloveHand { get { return hand; } }

        /*! \brief Update Skeletal with current date
        *
        * Retrieves the current Skeletal from the glove and
        * converts is from Manus to Unity format.
        *
        * \param unitySkel Skeletal as generated by Unity, which is to be updated.
        */
        public void UpdateSkeletal(ref Skeletal unitySkel){
            Debug.Log("UpdateSkeletal");
            GLOVE_SKELETAL manusSkel = new GLOVE_SKELETAL();
            ManusGetSkeletal(hand, ref manusSkel);
            
            ManusToUnity(ref unitySkel.Palm, manusSkel.palm);
            ManusToUnity(ref unitySkel.Index, manusSkel.index);
            ManusToUnity(ref unitySkel.Middle, manusSkel.middle);
            ManusToUnity(ref unitySkel.Pinky, manusSkel.pinky);
            ManusToUnity(ref unitySkel.Ring, manusSkel.ring);
            ManusToUnity(ref unitySkel.Thumb, manusSkel.thumb);
        }


        /*! \brief Constructor
        *
        * Constructor for Glove class
        *
        *  \param gh Left or right glove
        */
        public Glove(GLOVE_HAND gh) {
            hand = gh;
            Debug.Log("Calling Init");
            ManusInit();
            Debug.Log("Creating thread object");
            getDataThread = new Thread(new ThreadStart(getDataThreadFunc));
            Debug.Log("Creating mutex object");
            getDataMutex = new Mutex(false);
        }


        /*! Start Data Retrieval Thread
        */
        public void StartData()
        {
            Debug.Log("Starting thread");
            getDataThread.Start();
        }

        /*! Stop Data Retrieval Thread
        */
        public void StopData()
        {
            Debug.Log("Stopping thread");
            getDataThread.Abort();
        }

        
        public void Unload()
        {
            Debug.Log("Calling ManusExit()");
            ManusExit();
            
        }
    }
}
