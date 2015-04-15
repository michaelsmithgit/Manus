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

import android.app.Service;
import android.bluetooth.*;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Binder;
import android.os.IBinder;
import android.os.ParcelUuid;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

/**
 * \defgroup Glove Manus Glove
 * @{
 */
public class Manus extends Service {
    // TODO: Acquire Manus VID/PID
    private static final int VENDOR_ID      = 0x0;
    private static final int PRODUCT_ID     = 0x0;
    private static final byte GLOVE_PAGE    = 0x03;
    private static final byte GLOVE_USAGE   = 0x04;

    // Binder given to clients
    private final IBinder mBinder = new GloveBinder();

    // List of detected gloves
    private List<Glove> mGloves = new ArrayList<>();

    // The current context
    private Context mContext = this;

    // Scheduler to detect newly bonded gloves
    private final ScheduledExecutorService mScheduler =
            Executors.newScheduledThreadPool(1);

    // Updates the list of gloves at a set interval
    private final Runnable mGloveUpdater = new Runnable() {
        public void run() {
            // Use this check to determine whether BLE is supported on the device.
            if (!getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
                throw new UnsupportedOperationException();
            }

            // Initializes a Bluetooth adapter.  For API level 18 and above, get a reference to
            // BluetoothAdapter through BluetoothManager.
            final BluetoothManager bluetoothManager =
                    (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);

            // Build the list of gloves
            Set<BluetoothDevice> devices = bluetoothManager.getAdapter().getBondedDevices();
            for (BluetoothDevice dev : devices) {
                // Skip this device if a glove has already been added for it
                for (Glove glove : mGloves) {
                    if (glove.mGatt.getDevice().getAddress().equals(dev.getAddress()))
                        continue;
                }

                // Check if the cached services contain an HID service
                for (ParcelUuid uuid : dev.getUuids()) {
                    if (uuid.getUuid().equals(Glove.HID_SERVICE)) {
                        mGloves.add(new Glove(mContext, dev));
                        break;
                    }
                }
            }
        }
    };

    /**
     * Class used for the client Binder.  Because we know this service always
     * runs in the same process as its clients, we don't need to deal with IPC.
     */
    public class GloveBinder extends Binder {
        public int getGloveCount() {
            return mGloves.size();
        }

        public Glove getGlove(int glove) {
            return mGloves.get(glove);
        }
    }

    @Override
    public void onCreate() {
        mScheduler.scheduleAtFixedRate(mGloveUpdater, 0, 1, TimeUnit.SECONDS);
    }

    @Override
    public void onDestroy() {
        for (Glove glove : mGloves)
            glove.mGatt.close();
        mGloves.clear();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }
}
