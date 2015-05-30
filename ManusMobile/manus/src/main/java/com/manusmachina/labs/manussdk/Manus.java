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
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.os.Binder;
import android.os.IBinder;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

/**
 * \defgroup Glove Manus Glove
 * @{
 */
public class Manus extends Service {
    // Binder given to clients
    private final IBinder mBinder = new GloveBinder();

    // The bluetooth adapter
    private BluetoothAdapter mAdapter = BluetoothAdapter.getDefaultAdapter();

    // List of detected gloves
    private List<Glove> mGloves = new ArrayList<>();

    // List of event listeners
    private ArrayList<OnGloveChangedListener> mOnGloveChangedListeners = new ArrayList<>();

    // Connect to a BluetoothDevice
    private void connect(BluetoothDevice dev) {
        // Reconnect to this device if a glove has already been added for it
        for (Glove glove : mGloves) {
            if (glove.mGatt.getDevice().getAddress().equals(dev.getAddress())) {
                glove.mGatt.connect();
                return;
            }
        }

        mGloves.add(new Glove(this, dev, mGloveCallback));
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();

            if (action.equals(BluetoothDevice.ACTION_BOND_STATE_CHANGED)) {
                final BluetoothDevice dev = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                final int state = intent.getIntExtra(BluetoothDevice.EXTRA_BOND_STATE,
                        BluetoothDevice.BOND_NONE);
                if (state == BluetoothDevice.BOND_BONDED) {
                    // Connect to the new bluetooth device
                    connect(dev);
                }
            } else if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
                final int state = intent.getIntExtra(BluetoothAdapter.EXTRA_CONNECTION_STATE,
                        BluetoothAdapter.ERROR);

                // Connect to all bonded devices
                if (state == BluetoothAdapter.STATE_ON && mAdapter != null) {
                    for (BluetoothDevice device : mAdapter.getBondedDevices())
                        connect(device);
                }
            }
        }
    };

    private GloveCallback mGloveCallback = new GloveCallback() {
        @Override
        public void OnChanged(Glove glove) {
            int index = mGloves.indexOf(glove);
            for (OnGloveChangedListener listener : mOnGloveChangedListeners) {
                listener.OnGloveChanged(index, glove);
            }
        }
    };

    public interface OnGloveChangedListener {
        public void OnGloveChanged(int index, Glove glove);
    }

    /**
     * Class used for the client Binder.  Because we know this service always
     * runs in the same process as its clients, we don't need to deal with IPC.
     */
    public class GloveBinder extends Binder {
        public int getGloveCount() {
            return mGloves.size();
        }

        public Glove getGlove(int glove) {
            if (glove < mGloves.size())
                return mGloves.get(glove);
            else
                return null;
        }

        /**
         * Add a listener that will be called when the glove state is changed.
         *
         * @param listener The listener that will be called when the glove state changes.
         */
        public void addOnGloveChangedListener(OnGloveChangedListener listener) {
            if (!mOnGloveChangedListeners.contains(listener)) {
                mOnGloveChangedListeners.add(listener);
            }
        }

        /**
         * Remove a listener for glove state changes.
         *
         * @param listener The listener for glove state changes.
         */
        public void removeOnGloveChangedListener(OnGloveChangedListener listener) {
            mOnGloveChangedListeners.remove(listener);
        }
    }

    @Override
    public void onCreate() {
        // Use this check to determine whether BLE is supported on the device.
        if (!getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
            throw new UnsupportedOperationException();
        }

        // Initializes a Bluetooth adapter.  For API level 18 and above, get a reference to
        // BluetoothAdapter through BluetoothManager.
        final BluetoothManager bluetoothManager =
                (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);

        // Retrieve the bluetooth adapter
        if (bluetoothManager != null) {
            mAdapter = bluetoothManager.getAdapter();
        }

        // Connect to all bonded devices
        if (mAdapter != null) {
            for (BluetoothDevice device : mAdapter.getBondedDevices())
                connect(device);
        }

        // Register for broadcasts
        IntentFilter filter = new IntentFilter();
        filter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
        filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
        registerReceiver(mReceiver, filter);
    }

    @Override
    public void onDestroy() {
        unregisterReceiver(mReceiver);

        for (Glove glove : mGloves) {
            glove.close();
        }
        mGloves.clear();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }
}
