// Copyright (c) 2014, Freescale Semiconductor, Inc.
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of Freescale Semiconductor, Inc. nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL FREESCALE SEMICONDUCTOR, INC. BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include "types.h"
#include "magnetic.h"

// *********************************************************************************
// COMPUTE_9DOF_GBY_KALMAN constants
// *********************************************************************************
// kalman filter noise variances
#define FQVA_9DOF_GBY_KALMAN 2E-6F				// accelerometer noise g^2 so 1.4mg RMS
#define FQVM_9DOF_GBY_KALMAN 0.1F				// magnetometer noise uT^2
#define FQVG_9DOF_GBY_KALMAN 0.3F				// gyro noise (deg/s)^2
#define FQWB_9DOF_GBY_KALMAN 1E-9F				// gyro offset drift (deg/s)^2: 1E-9 implies 0.09deg/s max at 50Hz
#define FQWA_9DOF_GBY_KALMAN 1E-4F				// linear acceleration drift g^2 (increase slows convergence to g but reduces sensitivity to shake)
#define FQWD_9DOF_GBY_KALMAN 0.5F				// magnetic disturbance drift uT^2 (increase slows convergence to B but reduces sensitivity to magnet)
// initialization of Qw covariance matrix
#define FQWINITTHTH_9DOF_GBY_KALMAN 2000E-5F	// th_e * th_e terms
#define FQWINITBB_9DOF_GBY_KALMAN 250E-3F		// b_e * b_e terms
#define FQWINITTHB_9DOF_GBY_KALMAN 0.0F			// th_e * b_e terms
#define FQWINITAA_9DOF_GBY_KALMAN 10E-5F		// a_e * a_e terms (increase slows convergence to g but reduces sensitivity to shake)
#define FQWINITDD_9DOF_GBY_KALMAN 600E-3F		// d_e * d_e terms (increase slows convergence to B but reduces sensitivity to magnet)
// linear acceleration and magnetic disturbance time constants
#define FCA_9DOF_GBY_KALMAN 0.5F				// linear acceleration decay factor
#define FCD_9DOF_GBY_KALMAN 0.5F				// magnetic disturbance decay factor
// maximum geomagnetic inclination angle tracked by Kalman filter
#define SINDELTAMAX 0.9063078F		// sin of max +ve geomagnetic inclination angle: here 65.0 deg
#define COSDELTAMAX 0.4226183F		// cos of max +ve geomagnetic inclination angle: here 65.0 deg

// useful multiplicative conversion constants
#define PI 3.141592654F					// Pi
#define FDEGTORAD 0.01745329251994F		// degrees to radians conversion = pi / 180
#define FRADTODEG 57.2957795130823F		// radians to degrees conversion = 180 / pi
#define FRECIP180 0.0055555555555F		// multiplicative factor 1/180
#define ONETHIRD 0.33333333F			// one third
#define ONESIXTH 0.166666667F			// one sixth
#define ONETWELFTH 0.0833333333F		// one twelfth
#define ONEOVER48 0.02083333333F		// 1 / 48
#define ONEOVER120 0.0083333333F		// 1 / 120
#define ONEOVER3840 0.0002604166667F	// 1 / 3840
#define ONEOVERROOT2 0.707106781F		// 1/sqrt(2)
#define ROOT3OVER2 0.866025403784F		// sqrt(3)/2

// coordinate system for the build
#define NED 0                       // identifier for NED angle output
#define ANDROID 1                   // identifier for Android angle output
#define WIN8 2						// identifier for Windows 8 angle output
#define THISCOORDSYSTEM ANDROID		// the coordinate system to be used

// geomagnetic model parameters
#define DEFAULTB 50.0F				// default geomagnetic field (uT)

// sampling rate and kalman filter timing
//#define FTM_INCLK_HZ		1000000		// int32: 1MHz FTM timer frequency set in PE: do not change
#define SENSORFS 			100         // int32: 200Hz: frequency (Hz) of sensor sampling process
#define OVERSAMPLE_RATIO 	1       	// int32: 8x: 3DOF, 6DOF, 9DOF run at SENSORFS / OVERSAMPLE_RATIO Hz

// quaternion structure definition
struct fquaternion
{
	float q0;	// scalar component
	float q1;	// x vector component
	float q2;	// y vector component
	float q3;	// z vector component
};

// accelerometer sensor structure definition
struct AccelSensor
{
	int iSumGpFast[3];	// sum of fast measurements
	float fGpFast[3];		// fast (typically 200Hz) readings (g)
	float fGp[3];			// slow (typically 25Hz) averaged readings (g)
	float fgPerCount;		// initialized to FGPERCOUNT
	int iGpFast[3];		// fast (typically 200Hz) readings
	int iGp[3];			// slow (typically 25Hz) averaged readings (counts)
};

// magnetometer sensor structure definition
struct MagSensor
{
	int iSumBpFast[3];	// sum of fast measurements
	float fBpFast[3];		// fast (typically 200Hz) raw readings (uT)
	float fBp[3];			// slow (typically 25Hz) averaged raw readings (uT)
	float fBcFast[3];		// fast (typically 200Hz) calibrated readings (uT)
	float fBc[3];			// slow (typically 25Hz) averaged calibrated readings (uT)
	float fuTPerCount;		// initialized to FUTPERCOUNT
	float fCountsPeruT;		// initialized to FCOUNTSPERUT
	int iBpFast[3];		// fast (typically 200Hz) raw readings (counts)
	int iBp[3];			// slow (typically 25Hz) averaged raw readings (counts)
	int iBc[3];			// slow (typically 25Hz) averaged calibrated readings (counts)
};

// gyro sensor structure definition
struct GyroSensor
{
	int iSumYpFast[3];					// sum of fast measurements
	float fYp[3];							// raw gyro sensor output (deg/s)
	float fDegPerSecPerCount;				// initialized to FDEGPERSECPERCOUNT
	int iYpFast[OVERSAMPLE_RATIO][3];		// fast (typically 200Hz) readings
	int iYp[3];							// averaged gyro sensor output (counts)
};

// 9DOF Kalman filter accelerometer, magnetometer and gyroscope state vector structure
struct SV_9DOF_GBY_KALMAN
{
	// start: elements common to all motion state vectors
	// Euler angles
	float fPhiPl;					// roll (deg)
	float fThePl;					// pitch (deg)
	float fPsiPl;					// yaw (deg)
	float fRhoPl;					// compass (deg)
	float fChiPl;					// tilt from vertical (deg)
	// orientation matrix, quaternion and rotation vector
	float fRPl[3][3];				// a posteriori orientation matrix
	struct fquaternion fqPl;		// a posteriori orientation quaternion
	float fRVecPl[3];				// rotation vector
	// angular velocity
	float fOmega[3];				// angular velocity (deg/s)
	// systick timer for benchmarking
	//int32 systick;					// systick timer;
	// end: elements common to all motion state vectors

	// elements transmitted over bluetooth in kalman packet
	float fbPl[3];					// gyro offset (deg/s)
	float fThErrPl[3];				// orientation error (deg)
	float fbErrPl[3];				// gyro offset error (deg/s)
	// end elements transmitted in kalman packet

	float fdErrGlPl[3];				// magnetic disturbance error (uT, global frame)
	float fdErrSePl[3];				// magnetic disturbance error (uT, sensor frame)
	float faErrSePl[3];				// linear acceleration error (g, sensor frame)
	float faSeMi[3];				// linear acceleration (g, sensor frame)
	float fDeltaPl;					// inclination angle (deg)
	float faSePl[3];				// linear acceleration (g, sensor frame)
	float faGlPl[3];				// linear acceleration (g, global frame)
	float fgErrSeMi[3];				// difference (g, sensor frame) of gravity vector (accel) and gravity vector (gyro)
	float fmErrSeMi[3];				// difference (uT, sensor frame) of geomagnetic vector (magnetometer) and geomagnetic vector (gyro)
	float fgSeGyMi[3];				// gravity vector (g, sensor frame) measurement from gyro
	float fmSeGyMi[3];				// geomagnetic vector (uT, sensor frame) measurement from gyro
	float fmGl[3];					// geomagnetic vector (uT, global frame)
	float fQvAA;					// accelerometer terms of Qv
	float fQvMM;					// magnetometer terms of Qv
	float fPPlus12x12[12][12];		// covariance matrix P+
	float fK12x6[12][6];			// kalman filter gain matrix K
	float fQw12x12[12][12];			// covariance matrix Qw
	float fC6x12[6][12];			// measurement matrix C
	float fRMi[3][3];				// a priori orientation matrix
	struct fquaternion fDeltaq;		// delta quaternion
	struct fquaternion fqMi;		// a priori orientation quaternion
	float fcasq;					// FCA * FCA;
	float fcdsq;					// FCD * FCD;
	float fFastdeltat;				// sensor sampling interval (s) = 1 / SENSORFS
	float fdeltat;					// kalman filter sampling interval (s) = OVERSAMPLE_RATIO / SENSORFS
	float fdeltatsq;				// fdeltat * fdeltat
	float fQwbplusQvG;				// FQWB + FQVG
	int16 iFirstOrientationLock;	// denotes that 9DOF orientation has locked to 6DOF
	int8 resetflag;					// flag to request re-initialization on next pass
};

class SensorFusion
{
public:
	SensorFusion();

	struct fquaternion Fusion_Task(struct AccelSensor *pthisAccel, struct MagSensor *pthisMag, struct fquaternion *pthisOrientMatrix);
private:
	void fInit_9DOF_GBY_KALMAN(struct SV_9DOF_GBY_KALMAN *pthisSV, int16 ithisCoordSystem, int16 iSensorFS, int16 iOverSampleRatio);
	void fRun_9DOF_GBY_KALMAN_MANUS(struct SV_9DOF_GBY_KALMAN *pthisSV, struct AccelSensor *pthisAccel, struct MagSensor *pthisMag, struct fquaternion *pthisOrientMatrix,
			struct MagCalibration *pthisMagCal, int16 ithisCoordSystem, int16 iOverSampleRatio);
	void MagCal_Run(struct MagCalibration *pthisMagCal, struct MagneticBuffer *pthisMagBuffer); // magnetic calibration
	void Fusion_Run();

	struct SV_9DOF_GBY_KALMAN thisSV_9DOF_GBY_KALMAN;
	struct MagSensor thisMag;					// this magnetometer
	struct MagCalibration thisMagCal;			// hard and soft iron magnetic calibration
	struct MagneticBuffer thisMagBuffer;		// magnetometer measurement buffer
	struct AccelSensor thisAccel;				// this accelerometer
	struct fquaternion thisOrientMatrix;		// this gyro quaternion
	uint32 loopcounter;
	uint8 MagCal_Event_Flag;					// 'global' flags
};
