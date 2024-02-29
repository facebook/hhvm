/*
 * Copyright 2013-2024 Real Logic Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package uk.co.real_logic.sbe;

import static uk.co.real_logic.sbe.PrimitiveValue.*;

/**
 * Primitive types from which all other types are composed.
 */
public enum PrimitiveType
{
    /**
     * One byte character type which is a flavour of ASCII.
     */
    CHAR("char", 1, MIN_VALUE_CHAR, MAX_VALUE_CHAR, NULL_VALUE_CHAR),

    /**
     * A raw byte or signed 8-bit integer.
     */
    INT8("int8", 1, MIN_VALUE_INT8, MAX_VALUE_INT8, NULL_VALUE_INT8),

    /**
     * A 16-bit signed integer.
     */
    INT16("int16", 2, MIN_VALUE_INT16, MAX_VALUE_INT16, NULL_VALUE_INT16),

    /**
     * A 32-bit signed integer.
     */
    INT32("int32", 4, MIN_VALUE_INT32, MAX_VALUE_INT32, NULL_VALUE_INT32),

    /**
     * A 64-bit signed integer.
     */
    INT64("int64", 8, MIN_VALUE_INT64, MAX_VALUE_INT64, NULL_VALUE_INT64),

    /**
     * A 8-bit unsigned integer.
     */
    UINT8("uint8", 1, MIN_VALUE_UINT8, MAX_VALUE_UINT8, NULL_VALUE_UINT8),

    /**
     * A 16-bit unsigned integer.
     */
    UINT16("uint16", 2, MIN_VALUE_UINT16, MAX_VALUE_UINT16, NULL_VALUE_UINT16),

    /**
     * A 32-bit unsigned integer.
     */
    UINT32("uint32", 4, MIN_VALUE_UINT32, MAX_VALUE_UINT32, NULL_VALUE_UINT32),

    /**
     * A 64-bit unsigned integer.
     */
    UINT64("uint64", 8, MIN_VALUE_UINT64, MAX_VALUE_UINT64, NULL_VALUE_UINT64),

    /**
     * A 32-bit single precision floating point number.
     */
    FLOAT("float", 4, MIN_VALUE_FLOAT, MAX_VALUE_FLOAT, NULL_VALUE_FLOAT),

    /**
     * A 64-bit double precision floating point number.
     */
    DOUBLE("double", 8, MIN_VALUE_DOUBLE, MAX_VALUE_DOUBLE, NULL_VALUE_DOUBLE);

    private static final PrimitiveType[] VALUES = PrimitiveType.values();

    private final String name;
    private final int size;
    private final PrimitiveValue minValue;
    private final PrimitiveValue maxValue;
    private final PrimitiveValue nullValue;

    PrimitiveType(final String name, final int size, final long minValue, final long maxValue, final long nullValue)
    {
        this.name = name;
        this.size = size;
        this.minValue = new PrimitiveValue(minValue, size);
        this.maxValue = new PrimitiveValue(maxValue, size);
        this.nullValue = new PrimitiveValue(nullValue, size);
    }

    PrimitiveType(
        final String name, final int size, final double minValue, final double maxValue, final double nullValue)
    {
        this.name = name;
        this.size = size;
        this.minValue = new PrimitiveValue(minValue, size);
        this.maxValue = new PrimitiveValue(maxValue, size);
        this.nullValue = new PrimitiveValue(nullValue, size);
    }

    /**
     * The name of the primitive type as a String.
     *
     * @return the name as a String
     */
    public String primitiveName()
    {
        return name;
    }

    /**
     * The encodedLength of the primitive type in octets.
     *
     * @return encodedLength (in octets) of the primitive type
     */
    public int size()
    {
        return size;
    }

    /**
     * The minValue of the primitive type.
     *
     * @return default minValue of the primitive type
     */
    public PrimitiveValue minValue()
    {
        return minValue;
    }

    /**
     * The maxValue of the primitive type.
     *
     * @return default maxValue of the primitive type
     */
    public PrimitiveValue maxValue()
    {
        return maxValue;
    }

    /**
     * The nullValue of the primitive type.
     *
     * @return default nullValue of the primitive type
     */
    public PrimitiveValue nullValue()
    {
        return nullValue;
    }

    /**
     * Is the type an unsigned type like in C.
     *
     * @param type to be tested.
     * @return true if unsigned otherwise false.
     */
    public static boolean isUnsigned(final PrimitiveType type)
    {
        switch (type)
        {
            case UINT8:
            case UINT16:
            case UINT32:
            case UINT64:
                return true;

            default:
                return false;
        }
    }

    /**
     * Lookup PrimitiveType by String name and return Enum.
     *
     * @param name of primitiveType to get
     * @return the {@link PrimitiveType} matching the name
     * @throws IllegalArgumentException if name not found
     */
    public static PrimitiveType get(final String name)
    {
        for (final PrimitiveType primitiveType : VALUES)
        {
            if (primitiveType.name.equals(name))
            {
                return primitiveType;
            }
        }

        throw new IllegalArgumentException("No PrimitiveType for name: " + name);
    }
}
