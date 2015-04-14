package com.manusmachina.labs.manussdk;

import java.util.UUID;

/**
 * Created by Armada on 14-4-2015.
 */
public class UUID16 {
    private static final UUID BASE = new UUID(0x1000L, 0x800000805F9B34FBL);

    public static UUID toUUID(byte mostSigBits, byte leastSigBits) {
        long sigBits = (mostSigBits << 8) | leastSigBits;
        return new UUID(BASE.getMostSignificantBits() | (sigBits << 32), BASE.getLeastSignificantBits());
    }

    public static UUID toUUID(int mostSigBits, int leastSigBits) {
        // Because Java doesn't allow you to declare literals as bytes.
        return toUUID((byte) mostSigBits, (byte) leastSigBits);
    }
}
