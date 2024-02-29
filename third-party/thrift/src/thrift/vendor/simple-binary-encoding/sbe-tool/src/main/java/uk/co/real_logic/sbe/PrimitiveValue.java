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

import java.io.UnsupportedEncodingException;
import java.math.BigInteger;
import java.util.Arrays;

import static java.lang.Double.doubleToLongBits;
import static java.nio.charset.Charset.forName;
import static java.nio.charset.StandardCharsets.US_ASCII;

/**
 * Class used to encapsulate values for primitives. Used for nullValue, minValue, maxValue, and constants.
 */
public class PrimitiveValue
{
    /**
     * Representation type used for the stored value.
     */
    public enum Representation
    {
        /**
         * Value is stored in a long value.
         */
        LONG,

        /**
         * Value is stored in a double value.
         */
        DOUBLE,

        /**
         * Value is stored in a byte[].
         */
        BYTE_ARRAY
    }

    /**
     * Minimum value representation for a char type.
     */
    public static final long MIN_VALUE_CHAR = 0x20;

    /**
     * Maximum value representation for a char type.
     */
    public static final long MAX_VALUE_CHAR = 0x7E;

    /**
     * Null value representation for a char type.
     */
    public static final long NULL_VALUE_CHAR = 0;

    /**
     * Minimum value representation for a signed 8-bit type.
     */
    public static final long MIN_VALUE_INT8 = -127;

    /**
     * Maximum value representation for a signed 8-bit type.
     */
    public static final long MAX_VALUE_INT8 = 127;

    /**
     * Null value representation for a signed 8-bit type.
     */
    public static final long NULL_VALUE_INT8 = -128;

    /**
     * Minimum value representation for an unsigned 8-bit type.
     */
    public static final long MIN_VALUE_UINT8 = 0;

    /**
     * Maximum value representation for an unsigned 8-bit type.
     */
    public static final long MAX_VALUE_UINT8 = 254;

    /**
     * Null value representation for an unsigned 8-bit type.
     */
    public static final long NULL_VALUE_UINT8 = 255;

    /**
     * Minimum value representation for a signed 16-bit type.
     */
    public static final long MIN_VALUE_INT16 = -32767;

    /**
     * Maximum value representation for a signed 16-bit type.
     */
    public static final long MAX_VALUE_INT16 = 32767;

    /**
     * Null value representation for a signed 16-bit type.
     */
    public static final long NULL_VALUE_INT16 = -32768;

    /**
     * Minimum value representation for an unsigned 16-bit type.
     */
    public static final long MIN_VALUE_UINT16 = 0;

    /**
     * Maximum value representation for an unsigned 16-bit type.
     */
    public static final long MAX_VALUE_UINT16 = 65534;

    /**
     * Null value representation for an unsigned 16-bit type.
     */
    public static final long NULL_VALUE_UINT16 = 65535;

    /**
     * Minimum value representation for a signed 32-bit type.
     */
    public static final long MIN_VALUE_INT32 = -2147483647;

    /**
     * Maximum value representation for a signed 32-bit type.
     */
    public static final long MAX_VALUE_INT32 = 2147483647;

    /**
     * Null value representation for a signed 32-bit type.
     */
    public static final long NULL_VALUE_INT32 = -2147483648;

    /**
     * Minimum value representation for an unsigned 32-bit type.
     */
    public static final long MIN_VALUE_UINT32 = 0;

    /**
     * Maximum value representation for an unsigned 32-bit type.
     */
    public static final long MAX_VALUE_UINT32 = 0xFFFF_FFFFL - 1;

    /**
     * Null value representation for an unsigned 32-bit type.
     */
    public static final long NULL_VALUE_UINT32 = 0xFFFF_FFFFL;

    /**
     * Minimum value representation for a signed 64-bit type.
     */
    public static final long MIN_VALUE_INT64 = Long.MIN_VALUE + 1; // (-2 ^ 63) + 1

    /**
     * Maximum value representation for a signed 64-bit type.
     */
    public static final long MAX_VALUE_INT64 = Long.MAX_VALUE;     // ( 2 ^ 63) - 1

    /**
     * Null value representation for a signed 64-bit type.
     */
    public static final long NULL_VALUE_INT64 = Long.MIN_VALUE;    // (-2 ^ 63)

    /**
     * Minimum value representation for an unsigned 64-bit type.
     */
    public static final long MIN_VALUE_UINT64 = 0;

    /**
     * Maximum value representation for an unsigned 64-bit type.
     */
    public static final BigInteger BI_MAX_VALUE_UINT64 = new BigInteger("18446744073709551614");

    /**
     * Maximum value representation for an unsigned 64-bit type.
     */
    public static final long MAX_VALUE_UINT64 = BI_MAX_VALUE_UINT64.longValue(); // (2 ^ 64) - 2

    /**
     * Null value representation for an unsigned 64-bit type.
     */
    public static final BigInteger BI_NULL_VALUE_UINT64 = new BigInteger("18446744073709551615");

    /**
     * Null value representation for an unsigned 64-bit type.
     */
    public static final long NULL_VALUE_UINT64 = BI_NULL_VALUE_UINT64.longValue(); // (2 ^ 64) - 1

    /**
     * Maximum value representation for a single precision 32-bit floating point type.
     */
    public static final float MIN_VALUE_FLOAT = Float.MIN_VALUE;

    /**
     * Maximum value representation for a single precision 32-bit floating point type.
     */
    public static final float MAX_VALUE_FLOAT = Float.MAX_VALUE;

    /**
     * Null value representation for a single precision 32-bit floating point type.
     */
    public static final float NULL_VALUE_FLOAT = Float.NaN;

    /**
     * Minimum value representation for a double precision 64-bit floating point type.
     */
    public static final double MIN_VALUE_DOUBLE = Double.MIN_VALUE;

    /**
     * Maximum value representation for a double precision 64-bit floating point type.
     */
    public static final double MAX_VALUE_DOUBLE = Double.MAX_VALUE;

    /**
     * Null value representation for a double precision 64-bit floating point type.
     */
    public static final double NULL_VALUE_DOUBLE = Double.NaN;

    private final Representation representation;
    private final long longValue;
    private final double doubleValue;
    private final byte[] bytesValue;
    private final String characterEncoding;
    private final int size;
    private final byte[] byteArrayValueForLong = new byte[1];

    /**
     * Construct and fill in value as a long.
     *
     * @param value in long format.
     * @param size  of the type in bytes.
     */
    public PrimitiveValue(final long value, final int size)
    {
        representation = Representation.LONG;
        longValue = value;
        doubleValue = 0.0;
        bytesValue = null;
        characterEncoding = null;
        this.size = size;
    }

    /**
     * Construct and fill in value as a long.
     *
     * @param value             in long format
     * @param characterEncoding of the char type.
     */
    public PrimitiveValue(final byte value, final String characterEncoding)
    {
        representation = Representation.LONG;
        longValue = value;
        doubleValue = 0.0;
        bytesValue = null;
        this.characterEncoding = characterEncoding;
        this.size = 1;
    }

    /**
     * Construct and fill in value as a double.
     *
     * @param value in double format.
     * @param size  of the type in bytes.
     */
    public PrimitiveValue(final double value, final int size)
    {
        representation = Representation.DOUBLE;
        longValue = 0;
        doubleValue = value;
        bytesValue = null;
        characterEncoding = null;
        this.size = size;
    }

    /**
     * Construct and fill in value as a byte array.
     *
     * @param value             as a byte array.
     * @param characterEncoding of the characters.
     * @param size              of string in characters.
     */
    public PrimitiveValue(final byte[] value, final String characterEncoding, final int size)
    {
        representation = Representation.BYTE_ARRAY;
        longValue = 0;
        doubleValue = 0.0;
        bytesValue = value;
        this.characterEncoding = characterEncoding;
        this.size = size;
    }

    /**
     * Get the {@link Representation} of the value.
     *
     * @return the {@link Representation} of the value.
     */
    public Representation representation()
    {
        return representation;
    }

    /**
     * Parse constant value string and set representation based on type
     *
     * @param value         expressed as a String.
     * @param primitiveType that this is supposed to be.
     * @return a new {@link PrimitiveValue} for the value.
     * @throws IllegalArgumentException if parsing malformed type
     */
    public static PrimitiveValue parse(final String value, final PrimitiveType primitiveType)
    {
        switch (primitiveType)
        {
            case CHAR:
                if (value.length() > 1)
                {
                    throw new IllegalArgumentException("Constant char value malformed: " + value);
                }
                return new PrimitiveValue(value.getBytes(US_ASCII)[0], 1);

            case INT8:
                return new PrimitiveValue(Byte.parseByte(value), 1);

            case INT16:
                return new PrimitiveValue(Short.parseShort(value), 2);

            case INT32:
                return new PrimitiveValue(Integer.parseInt(value), 4);

            case INT64:
                return new PrimitiveValue(Long.parseLong(value), 8);

            case UINT8:
                return new PrimitiveValue(Short.parseShort(value), 1);

            case UINT16:
                return new PrimitiveValue(Integer.parseInt(value), 2);

            case UINT32:
                return new PrimitiveValue(Long.parseLong(value), 4);

            case UINT64:
                final BigInteger biValue = new BigInteger(value);
                if (biValue.compareTo(BI_NULL_VALUE_UINT64) > 0)
                {
                    throw new IllegalArgumentException("Value greater than UINT64 allows: value=" + value);
                }
                return new PrimitiveValue(biValue.longValue(), 8);

            case FLOAT:
                return new PrimitiveValue(Float.parseFloat(value), 4);

            case DOUBLE:
                return new PrimitiveValue(Double.parseDouble(value), 8);

            default:
                throw new IllegalArgumentException("Unknown PrimitiveType: " + primitiveType);
        }
    }

    /**
     * Parse constant value string and set representation based on type
     *
     * @param value             expressed as a String.
     * @param primitiveType     that this is supposed to be.
     * @param characterEncoding of the constant value.
     * @return a new {@link PrimitiveValue} for the value.
     * @throws IllegalArgumentException if parsing malformed type
     */
    public static PrimitiveValue parse(
        final String value, final PrimitiveType primitiveType, final String characterEncoding)
    {
        if (PrimitiveType.CHAR != primitiveType)
        {
            throw new IllegalArgumentException("primitiveType must be char: " + primitiveType);
        }

        if (value.length() > 1)
        {
            throw new IllegalArgumentException("Constant char value malformed: " + value);
        }

        return new PrimitiveValue(value.getBytes(forName(characterEncoding))[0], characterEncoding);
    }

    /**
     * Parse constant value string and set representation based on type, length, and characterEncoding
     *
     * @param value             expressed as a String.
     * @param length            of the type.
     * @param characterEncoding of the String.
     * @return a new {@link PrimitiveValue} for the value.
     * @throws IllegalArgumentException if parsing malformed type.
     */
    public static PrimitiveValue parse(
        final String value, final int length, final String characterEncoding)
    {
        if (value.length() > length)
        {
            throw new IllegalStateException("value.length=" + value.length() + " greater than length=" + length);
        }

        byte[] bytes = value.getBytes(forName(characterEncoding));
        if (bytes.length < length)
        {
            bytes = Arrays.copyOf(bytes, length);
        }

        return new PrimitiveValue(bytes, characterEncoding, length);
    }

    /**
     * Return long value for this PrimitiveValue.
     *
     * @return value expressed as a long.
     * @throws IllegalStateException if not a long value representation.
     */
    public long longValue()
    {
        if (representation != Representation.LONG)
        {
            throw new IllegalStateException(
                "Not a long representation: representation=" + representation + " value=" + this);
        }

        return longValue;
    }

    /**
     * Return double value for this PrimitiveValue.
     *
     * @return value expressed as a double.
     * @throws IllegalStateException if not a double value representation.
     */
    public double doubleValue()
    {
        if (representation != Representation.DOUBLE)
        {
            throw new IllegalStateException(
                "Not a double representation: representation=" + representation + " value=" + this);
        }

        return doubleValue;
    }

    /**
     * Return byte array value for this PrimitiveValue.
     *
     * @return value expressed as a byte array.
     * @throws IllegalStateException if not a byte array value representation.
     */
    public byte[] byteArrayValue()
    {
        if (representation != Representation.BYTE_ARRAY)
        {
            throw new IllegalStateException(
                "Not a byte[] representation: representation=" + representation + " value=" + this);
        }

        return bytesValue;
    }

    /**
     * Return byte array value for this PrimitiveValue given a particular type
     *
     * @param type of this value.
     * @return value expressed as a byte array.
     * @throws IllegalStateException if not a byte array value representation.
     */
    public byte[] byteArrayValue(final PrimitiveType type)
    {
        if (representation == Representation.BYTE_ARRAY)
        {
            return bytesValue;
        }
        else if (representation == Representation.LONG && size == 1 && type == PrimitiveType.CHAR)
        {
            byteArrayValueForLong[0] = (byte)longValue;
            return byteArrayValueForLong;
        }

        throw new IllegalStateException("PrimitiveValue is not a byte[] representation");
    }

    /**
     * Return encodedLength for this PrimitiveValue for serialization purposes.
     *
     * @return encodedLength for serialization.
     */
    public int size()
    {
        return size;
    }

    /**
     * The character encoding of the byte array representation.
     *
     * @return the character encoding of te byte array representation.
     */
    public String characterEncoding()
    {
        return characterEncoding;
    }

    /**
     * Return String value representation of this object.
     *
     * @return String representing object value.
     */
    public String toString()
    {
        if (Representation.LONG == representation)
        {
            return Long.toString(longValue);
        }
        else if (Representation.DOUBLE == representation)
        {
            return Double.toString(doubleValue);
        }
        else if (Representation.BYTE_ARRAY == representation)
        {
            try
            {
                return null == characterEncoding ? new String(bytesValue) : new String(bytesValue, characterEncoding);
            }
            catch (final UnsupportedEncodingException ex)
            {
                throw new IllegalStateException(ex);
            }
        }
        else
        {
            throw new IllegalStateException("Unsupported Representation: " + representation);
        }
    }

    /**
     * Determine if two values are equivalent.
     *
     * @param value to compare this value with.
     * @return equivalence of values.
     */
    public boolean equals(final Object value)
    {
        if (value instanceof PrimitiveValue)
        {
            final PrimitiveValue rhs = (PrimitiveValue)value;

            if (representation == rhs.representation)
            {
                switch (representation)
                {
                    case LONG:
                        return longValue == rhs.longValue;

                    case DOUBLE:
                        return doubleToLongBits(doubleValue) == doubleToLongBits(rhs.doubleValue);

                    case BYTE_ARRAY:
                        return Arrays.equals(bytesValue, rhs.bytesValue);
                }
            }
        }

        return false;
    }

    /**
     * Return hashCode for value. This is the underlying representations hashCode for the value.
     *
     * @return int value of the hashCode.
     */
    public int hashCode()
    {
        final long bits;
        switch (representation)
        {
            case LONG:
                bits = longValue;
                break;

            case DOUBLE:
                bits = doubleToLongBits(doubleValue);
                break;

            case BYTE_ARRAY:
                return Arrays.hashCode(bytesValue);

            default:
                throw new IllegalStateException("Unrecognised representation: " + representation);
        }

        return (int)(bits ^ (bits >>> 32));
    }
}
