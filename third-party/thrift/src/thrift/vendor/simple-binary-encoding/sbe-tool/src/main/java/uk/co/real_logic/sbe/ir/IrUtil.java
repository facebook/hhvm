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
package uk.co.real_logic.sbe.ir;

import org.agrona.DirectBuffer;
import org.agrona.MutableDirectBuffer;
import uk.co.real_logic.sbe.PrimitiveType;
import uk.co.real_logic.sbe.PrimitiveValue;
import uk.co.real_logic.sbe.ir.generated.ByteOrderCodec;
import uk.co.real_logic.sbe.ir.generated.PresenceCodec;
import uk.co.real_logic.sbe.ir.generated.PrimitiveTypeCodec;
import uk.co.real_logic.sbe.ir.generated.SignalCodec;

import java.io.UnsupportedEncodingException;
import java.nio.ByteOrder;

/**
 * Utility functions for working with {@link Ir}.
 */
class IrUtil
{
    static final byte[] EMPTY_BUFFER = new byte[0];

    static ByteOrderCodec mapByteOrder(final ByteOrder byteOrder)
    {
        if (byteOrder == ByteOrder.BIG_ENDIAN)
        {
            return ByteOrderCodec.SBE_BIG_ENDIAN;
        }

        return ByteOrderCodec.SBE_LITTLE_ENDIAN;
    }

    static ByteOrder mapByteOrder(final ByteOrderCodec byteOrder)
    {
        switch (byteOrder)
        {
            case SBE_BIG_ENDIAN:
                return ByteOrder.BIG_ENDIAN;

            default:
            case SBE_LITTLE_ENDIAN:
            case NULL_VAL:
                return ByteOrder.LITTLE_ENDIAN;
        }
    }

    static SignalCodec mapSignal(final Signal signal)
    {
        switch (signal)
        {
            case BEGIN_MESSAGE:
                return SignalCodec.BEGIN_MESSAGE;

            case END_MESSAGE:
                return SignalCodec.END_MESSAGE;

            case BEGIN_FIELD:
                return SignalCodec.BEGIN_FIELD;

            case END_FIELD:
                return SignalCodec.END_FIELD;

            case BEGIN_COMPOSITE:
                return SignalCodec.BEGIN_COMPOSITE;

            case END_COMPOSITE:
                return SignalCodec.END_COMPOSITE;

            case BEGIN_ENUM:
                return SignalCodec.BEGIN_ENUM;

            case END_ENUM:
                return SignalCodec.END_ENUM;

            case BEGIN_SET:
                return SignalCodec.BEGIN_SET;

            case END_SET:
                return SignalCodec.END_SET;

            case BEGIN_GROUP:
                return SignalCodec.BEGIN_GROUP;

            case END_GROUP:
                return SignalCodec.END_GROUP;

            case BEGIN_VAR_DATA:
                return SignalCodec.BEGIN_VAR_DATA;

            case END_VAR_DATA:
                return SignalCodec.END_VAR_DATA;

            case VALID_VALUE:
                return SignalCodec.VALID_VALUE;

            case CHOICE:
                return SignalCodec.CHOICE;

            case ENCODING:
            default:
                return SignalCodec.ENCODING;
        }
    }

    static Signal mapSignal(final SignalCodec signal)
    {
        switch (signal)
        {
            case BEGIN_MESSAGE:
                return Signal.BEGIN_MESSAGE;

            case END_MESSAGE:
                return Signal.END_MESSAGE;

            case BEGIN_FIELD:
                return Signal.BEGIN_FIELD;

            case END_FIELD:
                return Signal.END_FIELD;

            case BEGIN_COMPOSITE:
                return Signal.BEGIN_COMPOSITE;

            case END_COMPOSITE:
                return Signal.END_COMPOSITE;

            case BEGIN_ENUM:
                return Signal.BEGIN_ENUM;

            case END_ENUM:
                return Signal.END_ENUM;

            case BEGIN_SET:
                return Signal.BEGIN_SET;

            case END_SET:
                return Signal.END_SET;

            case BEGIN_GROUP:
                return Signal.BEGIN_GROUP;

            case END_GROUP:
                return Signal.END_GROUP;

            case BEGIN_VAR_DATA:
                return Signal.BEGIN_VAR_DATA;

            case END_VAR_DATA:
                return Signal.END_VAR_DATA;

            case VALID_VALUE:
                return Signal.VALID_VALUE;

            case CHOICE:
                return Signal.CHOICE;

            case ENCODING:
            default:
                return Signal.ENCODING;
        }
    }

    static PrimitiveTypeCodec mapPrimitiveType(final PrimitiveType type)
    {
        if (type == null)
        {
            return PrimitiveTypeCodec.NONE;
        }

        switch (type)
        {
            case INT8:
                return PrimitiveTypeCodec.INT8;

            case INT16:
                return PrimitiveTypeCodec.INT16;

            case INT32:
                return PrimitiveTypeCodec.INT32;

            case INT64:
                return PrimitiveTypeCodec.INT64;

            case UINT8:
                return PrimitiveTypeCodec.UINT8;

            case UINT16:
                return PrimitiveTypeCodec.UINT16;

            case UINT32:
                return PrimitiveTypeCodec.UINT32;

            case UINT64:
                return PrimitiveTypeCodec.UINT64;

            case FLOAT:
                return PrimitiveTypeCodec.FLOAT;

            case DOUBLE:
                return PrimitiveTypeCodec.DOUBLE;

            case CHAR:
                return PrimitiveTypeCodec.CHAR;

            default:
                return PrimitiveTypeCodec.NONE;
        }
    }

    static PrimitiveType mapPrimitiveType(final PrimitiveTypeCodec type)
    {
        switch (type)
        {
            case INT8:
                return PrimitiveType.INT8;

            case INT16:
                return PrimitiveType.INT16;

            case INT32:
                return PrimitiveType.INT32;

            case INT64:
                return PrimitiveType.INT64;

            case UINT8:
                return PrimitiveType.UINT8;

            case UINT16:
                return PrimitiveType.UINT16;

            case UINT32:
                return PrimitiveType.UINT32;

            case UINT64:
                return PrimitiveType.UINT64;

            case FLOAT:
                return PrimitiveType.FLOAT;

            case DOUBLE:
                return PrimitiveType.DOUBLE;

            case CHAR:
                return PrimitiveType.CHAR;

            case NONE:
            default:
                return null;
        }
    }

    static int put(final MutableDirectBuffer buffer, final PrimitiveValue value, final PrimitiveType type)
    {
        if (value == null)
        {
            return 0;
        }

        switch (type)
        {
            case CHAR:
                if (value.size() == 1)
                {
                    if (value.representation() == PrimitiveValue.Representation.LONG)
                    {
                        buffer.putByte(0, (byte)value.longValue());
                    }
                    else
                    {
                        buffer.putByte(0, value.byteArrayValue()[0]);
                    }
                    return 1;
                }
                else
                {
                    buffer.putBytes(0, value.byteArrayValue(), 0, value.byteArrayValue().length);
                    return value.byteArrayValue().length;
                }

            case INT8:
                buffer.putByte(0, (byte)value.longValue());
                return 1;

            case INT16:
                buffer.putShort(0, (short)value.longValue(), ByteOrder.LITTLE_ENDIAN);
                return 2;

            case INT32:
                buffer.putInt(0, (int)value.longValue(), ByteOrder.LITTLE_ENDIAN);
                return 4;

            case INT64:
                buffer.putLong(0, value.longValue(), ByteOrder.LITTLE_ENDIAN);
                return 8;

            case UINT8:
                buffer.putByte(0, (byte)value.longValue());
                return 1;

            case UINT16:
                buffer.putShort(0, (short)value.longValue(), ByteOrder.LITTLE_ENDIAN);
                return 2;

            case UINT32:
                buffer.putInt(0, (int)value.longValue(), ByteOrder.LITTLE_ENDIAN);
                return 4;

            case UINT64:
                buffer.putLong(0, value.longValue(), ByteOrder.LITTLE_ENDIAN);
                return 8;

            case FLOAT:
                buffer.putFloat(0, (float)value.doubleValue(), ByteOrder.LITTLE_ENDIAN);
                return 4;

            case DOUBLE:
                buffer.putDouble(0, value.doubleValue(), ByteOrder.LITTLE_ENDIAN);
                return 8;

            default:
                return 0;
        }
    }

    static PrimitiveValue get(final DirectBuffer buffer, final PrimitiveType type, final int length)
    {
        if (length == 0)
        {
            return null;
        }

        switch (type)
        {
            case CHAR:
                if (length == 1)
                {
                    return new PrimitiveValue(buffer.getByte(0), "US-ASCII");
                }
                else
                {
                    final byte[] array = new byte[length];
                    buffer.getBytes(0, array, 0, array.length);
                    return new PrimitiveValue(array, "US-ASCII", array.length);
                }

            case INT8:
                return new PrimitiveValue(buffer.getByte(0), 1);

            case INT16:
                return new PrimitiveValue(buffer.getShort(0, ByteOrder.LITTLE_ENDIAN), 2);

            case INT32:
                return new PrimitiveValue(buffer.getInt(0, ByteOrder.LITTLE_ENDIAN), 4);

            case INT64:
                return new PrimitiveValue(buffer.getLong(0, ByteOrder.LITTLE_ENDIAN), 8);

            case UINT8:
                return new PrimitiveValue((short)(buffer.getByte(0) & 0xFF), 1);

            case UINT16:
                return new PrimitiveValue(buffer.getShort(0, ByteOrder.LITTLE_ENDIAN) & 0xFFFF, 2);

            case UINT32:
                return new PrimitiveValue(buffer.getInt(0, ByteOrder.LITTLE_ENDIAN) & 0xFFFF_FFFFL, 4);

            case UINT64:
                return new PrimitiveValue(buffer.getLong(0, ByteOrder.LITTLE_ENDIAN), 8);

            case FLOAT:
                return new PrimitiveValue(buffer.getFloat(0, ByteOrder.LITTLE_ENDIAN), 4);

            case DOUBLE:
                return new PrimitiveValue(buffer.getDouble(0, ByteOrder.LITTLE_ENDIAN), 8);

            default:
                return null;
        }
    }

    static byte[] getBytes(final String value, final String characterEncoding)
        throws UnsupportedEncodingException
    {
        if (null == value)
        {
            return EMPTY_BUFFER;
        }

        return value.getBytes(characterEncoding);
    }


    static Encoding.Presence mapPresence(final PresenceCodec presence)
    {
        switch (presence)
        {
            case SBE_OPTIONAL:
                return Encoding.Presence.OPTIONAL;

            case SBE_CONSTANT:
                return Encoding.Presence.CONSTANT;

            default:
            case SBE_REQUIRED:
                return Encoding.Presence.REQUIRED;
        }
    }

    static PresenceCodec mapPresence(final Encoding.Presence presence)
    {
        switch (presence)
        {
            case OPTIONAL:
                return PresenceCodec.SBE_OPTIONAL;

            case CONSTANT:
                return PresenceCodec.SBE_CONSTANT;

            default:
            case REQUIRED:
                return PresenceCodec.SBE_REQUIRED;
        }
    }
}
