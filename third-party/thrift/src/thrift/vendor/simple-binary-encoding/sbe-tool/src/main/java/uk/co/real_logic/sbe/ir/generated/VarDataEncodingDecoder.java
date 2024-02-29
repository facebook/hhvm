/* Generated SBE (Simple Binary Encoding) message codec. */
package uk.co.real_logic.sbe.ir.generated;

import org.agrona.DirectBuffer;


/**
 * Header for var data fields.
 */
@SuppressWarnings("all")
public final class VarDataEncodingDecoder
{
    public static final int SCHEMA_ID = 1;
    public static final int SCHEMA_VERSION = 0;
    public static final String SEMANTIC_VERSION = "";
    public static final int ENCODED_LENGTH = -1;
    public static final java.nio.ByteOrder BYTE_ORDER = java.nio.ByteOrder.LITTLE_ENDIAN;

    private int offset;
    private DirectBuffer buffer;

    public VarDataEncodingDecoder wrap(final DirectBuffer buffer, final int offset)
    {
        if (buffer != this.buffer)
        {
            this.buffer = buffer;
        }
        this.offset = offset;

        return this;
    }

    public DirectBuffer buffer()
    {
        return buffer;
    }

    public int offset()
    {
        return offset;
    }

    public int encodedLength()
    {
        return ENCODED_LENGTH;
    }

    public int sbeSchemaId()
    {
        return SCHEMA_ID;
    }

    public int sbeSchemaVersion()
    {
        return SCHEMA_VERSION;
    }

    public static int lengthEncodingOffset()
    {
        return 0;
    }

    public static int lengthEncodingLength()
    {
        return 2;
    }

    public static int lengthSinceVersion()
    {
        return 0;
    }

    public static int lengthNullValue()
    {
        return 65535;
    }

    public static int lengthMinValue()
    {
        return 0;
    }

    public static int lengthMaxValue()
    {
        return 65534;
    }

    public int length()
    {
        return (buffer.getShort(offset + 0, java.nio.ByteOrder.LITTLE_ENDIAN) & 0xFFFF);
    }


    public static int varDataEncodingOffset()
    {
        return 2;
    }

    public static int varDataEncodingLength()
    {
        return -1;
    }

    public static int varDataSinceVersion()
    {
        return 0;
    }

    public static short varDataNullValue()
    {
        return (short)255;
    }

    public static short varDataMinValue()
    {
        return (short)0;
    }

    public static short varDataMaxValue()
    {
        return (short)254;
    }

    public String toString()
    {
        if (null == buffer)
        {
            return "";
        }

        return appendTo(new StringBuilder()).toString();
    }

    public StringBuilder appendTo(final StringBuilder builder)
    {
        if (null == buffer)
        {
            return builder;
        }

        builder.append('(');
        builder.append("length=");
        builder.append(this.length());
        builder.append('|');
        builder.append(')');

        return builder;
    }
}
