/* Generated SBE (Simple Binary Encoding) message codec. */
package uk.co.real_logic.sbe.ir.generated;

import org.agrona.MutableDirectBuffer;


/**
 * Message identifiers and length of message root.
 */
@SuppressWarnings("all")
public final class MessageHeaderEncoder
{
    public static final int SCHEMA_ID = 1;
    public static final int SCHEMA_VERSION = 0;
    public static final String SEMANTIC_VERSION = "";
    public static final int ENCODED_LENGTH = 8;
    public static final java.nio.ByteOrder BYTE_ORDER = java.nio.ByteOrder.LITTLE_ENDIAN;

    private int offset;
    private MutableDirectBuffer buffer;

    public MessageHeaderEncoder wrap(final MutableDirectBuffer buffer, final int offset)
    {
        if (buffer != this.buffer)
        {
            this.buffer = buffer;
        }
        this.offset = offset;

        return this;
    }

    public MutableDirectBuffer buffer()
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

    public static int blockLengthEncodingOffset()
    {
        return 0;
    }

    public static int blockLengthEncodingLength()
    {
        return 2;
    }

    public static int blockLengthNullValue()
    {
        return 65535;
    }

    public static int blockLengthMinValue()
    {
        return 0;
    }

    public static int blockLengthMaxValue()
    {
        return 65534;
    }

    public MessageHeaderEncoder blockLength(final int value)
    {
        buffer.putShort(offset + 0, (short)value, java.nio.ByteOrder.LITTLE_ENDIAN);
        return this;
    }


    public static int templateIdEncodingOffset()
    {
        return 2;
    }

    public static int templateIdEncodingLength()
    {
        return 2;
    }

    public static int templateIdNullValue()
    {
        return 65535;
    }

    public static int templateIdMinValue()
    {
        return 0;
    }

    public static int templateIdMaxValue()
    {
        return 65534;
    }

    public MessageHeaderEncoder templateId(final int value)
    {
        buffer.putShort(offset + 2, (short)value, java.nio.ByteOrder.LITTLE_ENDIAN);
        return this;
    }


    public static int schemaIdEncodingOffset()
    {
        return 4;
    }

    public static int schemaIdEncodingLength()
    {
        return 2;
    }

    public static int schemaIdNullValue()
    {
        return 65535;
    }

    public static int schemaIdMinValue()
    {
        return 0;
    }

    public static int schemaIdMaxValue()
    {
        return 65534;
    }

    public MessageHeaderEncoder schemaId(final int value)
    {
        buffer.putShort(offset + 4, (short)value, java.nio.ByteOrder.LITTLE_ENDIAN);
        return this;
    }


    public static int versionEncodingOffset()
    {
        return 6;
    }

    public static int versionEncodingLength()
    {
        return 2;
    }

    public static int versionNullValue()
    {
        return 65535;
    }

    public static int versionMinValue()
    {
        return 0;
    }

    public static int versionMaxValue()
    {
        return 65534;
    }

    public MessageHeaderEncoder version(final int value)
    {
        buffer.putShort(offset + 6, (short)value, java.nio.ByteOrder.LITTLE_ENDIAN);
        return this;
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

        final MessageHeaderDecoder decoder = new MessageHeaderDecoder();
        decoder.wrap(buffer, offset);

        return decoder.appendTo(builder);
    }
}
