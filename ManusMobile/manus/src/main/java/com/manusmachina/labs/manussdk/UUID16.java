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

import java.util.UUID;

/**
 * Created by Armada on 14-4-2015.
 */
class UUID16 {
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
