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
package uk.co.real_logic.sbe.otf;

import org.agrona.DirectBuffer;
import uk.co.real_logic.sbe.PrimitiveType;
import uk.co.real_logic.sbe.PrimitiveValue;
import uk.co.real_logic.sbe.ir.Encoding;

import java.nio.ByteOrder;

/**
 * Utility functions for applying over encoded types to help with on-the-fly (OTF) decoding.
 */
public class Types
{
    /**
     * Get an integer value from a buffer at a given index for a {@link PrimitiveType}.
     *
     * @param buffer    from which to read.
     * @param index     at which the integer should be read.
     * @param type      of the integer encoded in the buffer.
     * @param byteOrder of the integer in the buffer.
     * @return the value of the encoded integer.
     */
    public static int getInt(
        final DirectBuffer buffer, final int index, final PrimitiveType type, final ByteOrder byteOrder)
    {
        switch (type)
        {
            case INT8:
                return buffer.getByte(index);

            case UINT8:
                return (short)(buffer.getByte(index) & 0xFF);

            case INT16:
                return buffer.getShort(index, byteOrder);

            case UINT16:
                return buffer.getShort(index, byteOrder) & 0xFFFF;

            case INT32:
                return buffer.getInt(index, byteOrder);

            case UINT32:
                final int value = buffer.getInt(index, byteOrder);
                if (value < 0)
                {
                    throw new IllegalStateException(
                        "UINT32 type should not be greater than Integer.MAX_VALUE: value=" + value);
                }
                return value;

            default:
                throw new IllegalArgumentException("Unsupported type: " + type);
        }
    }

    /**
     * Get a long value from a buffer at a given index for a given {@link Encoding}.
     *
     * @param buffer   from which to read.
     * @param index    at which the integer should be read.
     * @param encoding of the value.
     * @return the value of the encoded long.
     */
    public static long getLong(final DirectBuffer buffer, final int index, final Encoding encoding)
    {
        switch (encoding.primitiveType())
        {
            case CHAR:
                return buffer.getByte(index);

            case INT8:
                return buffer.getByte(index);

            case INT16:
                return buffer.getShort(index, encoding.byteOrder());

            case INT32:
                return buffer.getInt(index, encoding.byteOrder());

            case INT64:
                return buffer.getLong(index, encoding.byteOrder());

            case UINT8:
                return (short)(buffer.getByte(index) & 0xFF);

            case UINT16:
                return buffer.getShort(index, encoding.byteOrder()) & 0xFFFF;

            case UINT32:
                return buffer.getInt(index, encoding.byteOrder()) & 0xFFFF_FFFFL;

            case UINT64:
                return buffer.getLong(index, encoding.byteOrder());

            default:
                throw new IllegalArgumentException("Unsupported type for long: " + encoding.primitiveType());
        }
    }

    /**
     * Append an encoding as a String to a {@link StringBuilder}.
     *
     * @param sb       to append the encoding to.
     * @param buffer   containing the encoded value.
     * @param index    at which the encoded value exists.
     * @param encoding representing the encoded value.
     */
    public static void appendAsString(
        final StringBuilder sb, final DirectBuffer buffer, final int index, final Encoding encoding)
    {
        switch (encoding.primitiveType())
        {
            case CHAR:
                sb.append('\'').append((char)buffer.getByte(index)).append('\'');
                break;

            case INT8:
                sb.append(buffer.getByte(index));
                break;

            case INT16:
                sb.append(buffer.getShort(index, encoding.byteOrder()));
                break;

            case INT32:
                sb.append(buffer.getInt(index, encoding.byteOrder()));
                break;

            case INT64:
                sb.append(buffer.getLong(index, encoding.byteOrder()));
                break;

            case UINT8:
                sb.append((short)(buffer.getByte(index) & 0xFF));
                break;

            case UINT16:
                sb.append(buffer.getShort(index, encoding.byteOrder()) & 0xFFFF);
                break;

            case UINT32:
                sb.append(buffer.getInt(index, encoding.byteOrder()) & 0xFFFF_FFFFL);
                break;

            case UINT64:
                sb.append(buffer.getLong(index, encoding.byteOrder()));
                break;

            case FLOAT:
                sb.append(buffer.getFloat(index, encoding.byteOrder()));
                break;

            case DOUBLE:
                sb.append(buffer.getDouble(index, encoding.byteOrder()));
                break;
        }
    }

    /**
     * Append an encoding as a Json String to a {@link StringBuilder}.
     *
     * @param sb       to append the encoding to.
     * @param buffer   containing the encoded value.
     * @param index    at which the encoded value exists.
     * @param encoding representing the encoded value.
     */
    public static void appendAsJsonString(
        final StringBuilder sb, final DirectBuffer buffer, final int index, final Encoding encoding)
    {
        switch (encoding.primitiveType())
        {
            case CHAR:
                sb.append('\'').append((char)buffer.getByte(index)).append('\'');
                break;

            case INT8:
                sb.append(buffer.getByte(index));
                break;

            case INT16:
                sb.append(buffer.getShort(index, encoding.byteOrder()));
                break;

            case INT32:
                sb.append(buffer.getInt(index, encoding.byteOrder()));
                break;

            case INT64:
                sb.append(buffer.getLong(index, encoding.byteOrder()));
                break;

            case UINT8:
                sb.append((short)(buffer.getByte(index) & 0xFF));
                break;

            case UINT16:
                sb.append(buffer.getShort(index, encoding.byteOrder()) & 0xFFFF);
                break;

            case UINT32:
                sb.append(buffer.getInt(index, encoding.byteOrder()) & 0xFFFF_FFFFL);
                break;

            case UINT64:
                sb.append(buffer.getLong(index, encoding.byteOrder()));
                break;

            case FLOAT:
            {
                final float value = buffer.getFloat(index, encoding.byteOrder());
                if (Float.isNaN(value))
                {
                    sb.append("0/0");
                }
                else if (value == Float.POSITIVE_INFINITY)
                {
                    sb.append("1/0");
                }
                else if (value == Float.NEGATIVE_INFINITY)
                {
                    sb.append("-1/0");
                }
                else
                {
                    sb.append(value);
                }
                break;
            }

            case DOUBLE:
            {
                final double value = buffer.getDouble(index, encoding.byteOrder());
                if (Double.isNaN(value))
                {
                    sb.append("0/0");
                }
                else if (value == Double.POSITIVE_INFINITY)
                {
                    sb.append("1/0");
                }
                else if (value == Double.NEGATIVE_INFINITY)
                {
                    sb.append("-1/0");
                }
                else
                {
                    sb.append(value);
                }
                break;
            }
        }
    }

    /**
     * Append a value as a Json String to a {@link StringBuilder}.
     *
     * @param sb       to append the value to.
     * @param value    to append.
     * @param encoding representing the encoded value.
     */
    public static void appendAsJsonString(final StringBuilder sb, final PrimitiveValue value, final Encoding encoding)
    {
        switch (encoding.primitiveType())
        {
            case CHAR:
                sb.append('\'').append((char)value.longValue()).append('\'');
                break;

            case INT8:
            case UINT8:
            case INT16:
            case UINT16:
            case INT32:
            case UINT32:
            case INT64:
            case UINT64:
                sb.append(value.longValue());
                break;

            case FLOAT:
            {
                final float floatValue = (float)value.doubleValue();
                if (Float.isNaN(floatValue))
                {
                    sb.append("0/0");
                }
                else if (floatValue == Float.POSITIVE_INFINITY)
                {
                    sb.append("1/0");
                }
                else if (floatValue == Float.NEGATIVE_INFINITY)
                {
                    sb.append("-1/0");
                }
                else
                {
                    sb.append(value);
                }
                break;
            }

            case DOUBLE:
            {
                final double doubleValue = value.doubleValue();
                if (Double.isNaN(doubleValue))
                {
                    sb.append("0/0");
                }
                else if (doubleValue == Double.POSITIVE_INFINITY)
                {
                    sb.append("1/0");
                }
                else if (doubleValue == Double.NEGATIVE_INFINITY)
                {
                    sb.append("-1/0");
                }
                else
                {
                    sb.append(doubleValue);
                }
                break;
            }
        }
    }
}
