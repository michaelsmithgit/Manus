package com.manusmachina.labs.manussdk;

import java.io.Closeable;

/**
 * Created by Armada on 23-4-2015.
 */
class SensorFusion implements Closeable {
    static {
        System.loadLibrary("SensorFusion");
    }

    private long objectPtr;

    public SensorFusion() {
        objectPtr = init();
    }

    private native long init();
    public native void close();

    public native float[] fusion(float[] accel, float[] mag, float[] quat);
}
