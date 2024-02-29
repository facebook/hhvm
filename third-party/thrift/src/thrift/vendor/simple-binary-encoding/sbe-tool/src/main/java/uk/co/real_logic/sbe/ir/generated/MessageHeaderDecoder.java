/* Generated SBE (Simple Binary Encoding) message codec. */
package uk.co.real_logic.sbe.ir.generated;

import org.agrona.DirectBuffer;


/**
 * Message identifiers and length of message root.
 */
@SuppressWarnings("all")
public final class MessageHeaderDecoder
{
    public static final int SCHEMA_ID = 1;
    public static final int SCHEMA_VERSION = 0;
    public static final String SEMANTIC_VERSION = "";
    public static final int ENCODED_LENGTH = 8;
    public static final java.nio.ByteOrder BYTE_ORDER = java.nio.ByteOrder.LITTLE_ENDIAN;

    private int offset;
    private DirectBuffer buffer;

    public MessageHeaderDecoder wrap(final DirectBuffer buffer, final int offset)
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

    public static int blockLengthEncodingOffset()
    {
        return 0;
    }

    public static int blockLengthEncodingLength()
    {
        return 2;
    }

    public static int blockLengthSinceVersion()
    {
        return 0;
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

    public int blockLength()
    {
        return (buffer.getShort(offset + 0, java.nio.ByteOrder.LITTLE_ENDIAN) & 0xFFFF);
    }


    public static int templateIdEncodingOffset()
    {
        return 2;
    }

    public static int templateIdEncodingLength()
    {
        return 2;
    }

    public static int templateIdSinceVersion()
    {
        return 0;
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

    public int templateId()
    {
        return (buffer.getShort(offset + 2, java.nio.ByteOrder.LITTLE_ENDIAN) & 0xFFFF);
    }


    public static int schemaIdEncodingOffset()
    {
        return 4;
    }

    public static int schemaIdEncodingLength()
    {
        return 2;
    }

    public static int schemaIdSinceVersion()
    {
        return 0;
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

    public int schemaId()
    {
        return (buffer.getShort(offset + 4, java.nio.ByteOrder.LITTLE_ENDIAN) & 0xFFFF);
    }


    public static int versionEncodingOffset()
    {
        return 6;
    }

    public static int versionEncodingLength()
    {
        return 2;
    }

    public static int versionSinceVersion()
    {
        return 0;
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

    public int version()
    {
        return (buffer.getShort(offset + 6, java.nio.ByteOrder.LITTLE_ENDIAN) & 0xFFFF);
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
        builder.append("blockLength=");
        builder.append(this.blockLength());
        builder.append('|');
        builder.append("templateId=");
        builder.append(this.templateId());
        builder.append('|');
        builder.append("schemaId=");
        builder.append(this.schemaId());
        builder.append('|');
        builder.append("version=");
        builder.append(this.version());
        builder.append(')');

        return builder;
    }
}
