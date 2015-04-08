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

package com.manusmachina.labs.manussdk;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.content.Context;

import java.util.ArrayList;
import java.util.Observable;
import java.util.UUID;

/**
 * Created by Armada on 8-4-2015.
 */
// TODO: Make this an observable class
public class Glove {
    public class Quaternion {
        public float w, x, y, z;

        public Quaternion(float w, float x, float y, float z) {
            this.w = w;
            this.x = x;
            this.y = y;
            this.z = z;
        }
    }

    public class Vector {
        public float x, y, z;

        public Vector(float x, float y, float z) {
            this.x = x;
            this.y = y;
            this.z = z;
        }
    }

    private static final UUID HID_SERVICE       = new UUID(0x18, 0x12);
    private static final UUID HID_INFORMATION   = new UUID(0x2A, 0x4A);
    private static final UUID HID_REPORT_MAP    = new UUID(0x2A, 0x4B);
    private static final UUID HID_CONTROL_POINT = new UUID(0x2A, 0x4C);
    private static final UUID HID_REPORT        = new UUID(0x2A, 0x4D);

    private static final float ACCEL_DIVISOR    = 16384.0f;
    private static final float QUAT_DIVISOR     = 16384.0f;
    private static final float COMPASS_DIVISOR  = 32.0f;
    private static final float FINGER_DIVISOR   = 255.0f;

    private byte[] mReportMap;
    private ArrayList<BluetoothGattCharacteristic> mReports;

    protected BluetoothGatt mGatt;
    protected byte mPage;
    protected byte mUsage;

    // Implements callback methods for GATT events that the app cares about.  For example,
    // connection change and services discovered.
    private final BluetoothGattCallback mGattCallback = new BluetoothGattCallback() {
    };

    public Glove(Context con, BluetoothDevice dev) {
        mPage = mUsage = 0;
        mGatt = dev.connectGatt(con, true, mGattCallback);

        // Get the HID Service if it exists
        BluetoothGattService service = mGatt.getService(HID_SERVICE);
        if (service != null) {
            // Get the HID Report Map if there is one
            BluetoothGattCharacteristic gchar = service.getCharacteristic(HID_REPORT_MAP);

            if (gchar != null) {
                mReportMap = gchar.getValue();
                mPage = mReportMap[0];
                mUsage = mReportMap[1];
            }

            for (BluetoothGattCharacteristic report : service.getCharacteristics()) {
                if (report.getUuid() == HID_REPORT) {
                    if ((report.getProperties() & BluetoothGattCharacteristic.PROPERTY_NOTIFY) != 0)
                        mGatt.setCharacteristicNotification(report, true);
                    mReports.add(report);
                }
            }
        }
    }

    public void close() {
        mGatt.close();
    }

    /*! \brief Get the state of a glove.
    *
    *  \param glove The glove index.
    *  \param state Output variable to receive the state.
    *  \param blocking Wait until the glove returns a value.
    *
    *  \return True if the glove is connected, False if it is not.
    */
    Quaternion getQuaternion() {
        BluetoothGattCharacteristic report = mReports.get(0);
        int format = BluetoothGattCharacteristic.FORMAT_UINT16;

        return new Quaternion(
                report.getIntValue(format, 0) / QUAT_DIVISOR,
                report.getIntValue(format, 2) / QUAT_DIVISOR,
                report.getIntValue(format, 4) / QUAT_DIVISOR,
                report.getIntValue(format, 6) / QUAT_DIVISOR
        );
    }

    Vector getAcceleration() {
        BluetoothGattCharacteristic report = mReports.get(0);
        int format = BluetoothGattCharacteristic.FORMAT_UINT16;

        return new Vector(
                report.getIntValue(format, 8) / ACCEL_DIVISOR,
                report.getIntValue(format, 10) / ACCEL_DIVISOR,
                report.getIntValue(format, 12) / ACCEL_DIVISOR
        );
    }

    float getFinger(int i) {
        BluetoothGattCharacteristic report = mReports.get(0);
        return report.getIntValue(BluetoothGattCharacteristic.FORMAT_UINT8, 14 + i) / FINGER_DIVISOR;
    }

    /*! \brief Convert a Quaternion to Euler angles.
    *
    *  Returns the Quaternion as Yaw, Pitch and Roll angles
    *  relative to the Earth's gravity.
    *
    *  \param euler Output variable to receive the Euler angles.
    *  \param quaternion The quaternion to convert.
    */
    Vector getEuler(final Quaternion q) {
        return new Vector(
                // roll: (tilt left/right, about X axis)
                (float)Math.atan2(2 * (q.w * q.x + q.y * q.z), 1 - 2 * (q.x * q.x + q.y * q.y)),
                // pitch: (nose up/down, about Y axis)
                (float)Math.asin(2 * (q.w * q.y - q.z * q.x)),
                // yaw: (about Z axis)
                (float)Math.atan2(2 * (q.w * q.z + q.x * q.y), 1 - 2 * (q.y * q.y + q.z * q.z))
        );
    }

    /*! \brief Remove gravity from acceleration vector.
    *
    *  Returns the Acceleration as a vector independent from
    *  the Earth's gravity.
    *
    *  \param linear Output vector to receive the linear acceleration.
    *  \param acceleation The acceleration vector to convert.
    */
    Vector getLinearAcceleration(final Vector vRaw, final Vector gravity) {
        return new Vector(
                vRaw.x - gravity.x,
                vRaw.y - gravity.y,
                vRaw.z - gravity.z
        );
    }

    /*! \brief Return gravity vector from the Quaternion.
    *
    *  Returns an estimation of the Earth's gravity vector.
    *
    *  \param gravity Output vector to receive the gravity vector.
    *  \param quaternion The quaternion to base the gravity vector on.
    */
    Vector getGravity(Quaternion q) {
        return new Vector(
                2 * (q.x*q.z - q.w*q.y),
                2 * (q.w*q.x + q.y*q.z),
                q.w*q.w - q.x*q.x - q.y*q.y + q.z*q.z
        );
    }
}
