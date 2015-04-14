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

import android.bluetooth.*;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.ParcelUuid;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.UUID;

/**
 * \defgroup Glove Manus Glove
 * @{
 */
public class Manus {
    // TODO: Acquire Manus VID/PID
    private static final int VENDOR_ID      = 0x0;
    private static final int PRODUCT_ID     = 0x0;
    private static final byte GLOVE_PAGE    = 0x03;
    private static final byte GLOVE_USAGE   = 0x04;

    public static List<Glove> getGloves(Context con) {
        // Use this check to determine whether BLE is supported on the device.
        if (!con.getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
            throw new UnsupportedOperationException();
        }

        // Initializes a Bluetooth adapter.  For API level 18 and above, get a reference to
        // BluetoothAdapter through BluetoothManager.
        final BluetoothManager bluetoothManager =
                (BluetoothManager) con.getSystemService(Context.BLUETOOTH_SERVICE);

        // Build the list of gloves
        Set<BluetoothDevice> devices = bluetoothManager.getAdapter().getBondedDevices();
        List<Glove> gloves = new ArrayList<>();
        for (BluetoothDevice dev : devices) {
            // Check if the cached services contain an HID service
            for (ParcelUuid uuid : dev.getUuids()) {
                if (uuid.getUuid().equals(Glove.HID_SERVICE)) {
                    Glove glove = new Glove(con, dev);
                    gloves.add(glove);
                }
            }
        }

        return gloves;
    }
}
