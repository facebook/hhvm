/* Generated SBE (Simple Binary Encoding) message codec. */
package uk.co.real_logic.sbe.ir.generated;

import org.agrona.MutableDirectBuffer;
import org.agrona.DirectBuffer;


/**
 * Frame Header for start of encoding IR.
 */
@SuppressWarnings("all")
public final class FrameCodecDecoder
{
    private static final boolean ENABLE_BOUNDS_CHECKS = !Boolean.getBoolean("agrona.disable.bounds.checks");

    private static final boolean SBE_ENABLE_IR_PRECEDENCE_CHECKS = Boolean.parseBoolean(System.getProperty(
        "sbe.enable.ir.precedence.checks",
        Boolean.toString(ENABLE_BOUNDS_CHECKS)));

    /**
     * The states in which a encoder/decoder/codec can live.
     *
     * <p>The state machine diagram below, encoded in the dot language, describes
     * the valid state transitions according to the order in which fields may be
     * accessed safely. Tools such as PlantUML and Graphviz can render it.
     *
     * <pre>{@code
     *   digraph G {
     *       NOT_WRAPPED -> V0_BLOCK [label="  wrap(version=0)  "];
     *       V0_BLOCK -> V0_BLOCK [label="  irId(?)  "];
     *       V0_BLOCK -> V0_BLOCK [label="  irVersion(?)  "];
     *       V0_BLOCK -> V0_BLOCK [label="  schemaVersion(?)  "];
     *       V0_BLOCK -> V0_BLOCK [label="  packageNameLength()  "];
     *       V0_BLOCK -> V0_PACKAGENAME_DONE [label="  packageName(?)  "];
     *       V0_PACKAGENAME_DONE -> V0_PACKAGENAME_DONE [label="  namespaceNameLength()  "];
     *       V0_PACKAGENAME_DONE -> V0_NAMESPACENAME_DONE [label="  namespaceName(?)  "];
     *       V0_NAMESPACENAME_DONE -> V0_NAMESPACENAME_DONE [label="  semanticVersionLength()  "];
     *       V0_NAMESPACENAME_DONE -> V0_SEMANTICVERSION_DONE [label="  semanticVersion(?)  "];
     *   }
     * }</pre>
     */
    private static class CodecStates
    {
        private static final int NOT_WRAPPED = 0;
        private static final int V0_BLOCK = 1;
        private static final int V0_PACKAGENAME_DONE = 2;
        private static final int V0_NAMESPACENAME_DONE = 3;
        private static final int V0_SEMANTICVERSION_DONE = 4;

        private static final String[] STATE_NAME_LOOKUP =
        {
            "NOT_WRAPPED",
            "V0_BLOCK",
            "V0_PACKAGENAME_DONE",
            "V0_NAMESPACENAME_DONE",
            "V0_SEMANTICVERSION_DONE",
        };

        private static final String[] STATE_TRANSITIONS_LOOKUP =
        {
            "\"wrap(version=0)\"",
            "\"irId(?)\", \"irVersion(?)\", \"schemaVersion(?)\", \"packageNameLength()\", \"packageName(?)\"",
            "\"namespaceNameLength()\", \"namespaceName(?)\"",
            "\"semanticVersionLength()\", \"semanticVersion(?)\"",
            "",
        };

        private static String name(final int state)
        {
            return STATE_NAME_LOOKUP[state];
        }

        private static String transitions(final int state)
        {
            return STATE_TRANSITIONS_LOOKUP[state];
        }
    }

    private int codecState = CodecStates.NOT_WRAPPED;

    private int codecState()
    {
        return codecState;
    }

    private void codecState(int newState)
    {
        codecState = newState;
    }

    public static final int BLOCK_LENGTH = 12;
    public static final int TEMPLATE_ID = 1;
    public static final int SCHEMA_ID = 1;
    public static final int SCHEMA_VERSION = 0;
    public static final String SEMANTIC_VERSION = "";
    public static final java.nio.ByteOrder BYTE_ORDER = java.nio.ByteOrder.LITTLE_ENDIAN;

    private final FrameCodecDecoder parentMessage = this;
    private DirectBuffer buffer;
    private int offset;
    private int limit;
    int actingBlockLength;
    int actingVersion;

    public int sbeBlockLength()
    {
        return BLOCK_LENGTH;
    }

    public int sbeTemplateId()
    {
        return TEMPLATE_ID;
    }

    public int sbeSchemaId()
    {
        return SCHEMA_ID;
    }

    public int sbeSchemaVersion()
    {
        return SCHEMA_VERSION;
    }

    public String sbeSemanticType()
    {
        return "";
    }

    public DirectBuffer buffer()
    {
        return buffer;
    }

    public int offset()
    {
        return offset;
    }

    private void onWrap(final int actingVersion)
    {
        switch(actingVersion)
        {
            case 0:
                codecState(CodecStates.V0_BLOCK);
                break;
            default:
                codecState(CodecStates.V0_BLOCK);
                break;
        }
    }

    public FrameCodecDecoder wrap(
        final DirectBuffer buffer,
        final int offset,
        final int actingBlockLength,
        final int actingVersion)
    {
        if (buffer != this.buffer)
        {
            this.buffer = buffer;
        }
        this.offset = offset;
        this.actingBlockLength = actingBlockLength;
        this.actingVersion = actingVersion;
        limit(offset + actingBlockLength);

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onWrap(actingVersion);
        }

        return this;
    }

    public FrameCodecDecoder wrapAndApplyHeader(
        final DirectBuffer buffer,
        final int offset,
        final MessageHeaderDecoder headerDecoder)
    {
        headerDecoder.wrap(buffer, offset);

        final int templateId = headerDecoder.templateId();
        if (TEMPLATE_ID != templateId)
        {
            throw new IllegalStateException("Invalid TEMPLATE_ID: " + templateId);
        }

        return wrap(
            buffer,
            offset + MessageHeaderDecoder.ENCODED_LENGTH,
            headerDecoder.blockLength(),
            headerDecoder.version());
    }

    public FrameCodecDecoder sbeRewind()
    {
        return wrap(buffer, offset, actingBlockLength, actingVersion);
    }

    public int sbeDecodedLength()
    {
        final int currentLimit = limit();
        final int currentCodecState = codecState();
        sbeSkip();
        final int decodedLength = encodedLength();
        limit(currentLimit);

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            codecState(currentCodecState);
        }

        return decodedLength;
    }

    public int actingVersion()
    {
        return actingVersion;
    }

    public int encodedLength()
    {
        return limit - offset;
    }

    public int limit()
    {
        return limit;
    }

    public void limit(final int limit)
    {
        this.limit = limit;
    }

    public static int irIdId()
    {
        return 1;
    }

    public static int irIdSinceVersion()
    {
        return 0;
    }

    public static int irIdEncodingOffset()
    {
        return 0;
    }

    public static int irIdEncodingLength()
    {
        return 4;
    }

    public static String irIdMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    private void onIrIdAccessed()
    {
        if (codecState() == CodecStates.NOT_WRAPPED)
        {
            throw new IllegalStateException("Illegal field access order. " +
                "Cannot access field \"irId\" in state: " + CodecStates.name(codecState()) +
                ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                "]. Please see the diagram in the Javadoc of the class FrameCodecDecoder#CodecStates.");
        }
    }

    public static int irIdNullValue()
    {
        return -2147483648;
    }

    public static int irIdMinValue()
    {
        return -2147483647;
    }

    public static int irIdMaxValue()
    {
        return 2147483647;
    }

    public int irId()
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onIrIdAccessed();
        }

        return buffer.getInt(offset + 0, java.nio.ByteOrder.LITTLE_ENDIAN);
    }


    public static int irVersionId()
    {
        return 2;
    }

    public static int irVersionSinceVersion()
    {
        return 0;
    }

    public static int irVersionEncodingOffset()
    {
        return 4;
    }

    public static int irVersionEncodingLength()
    {
        return 4;
    }

    public static String irVersionMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    private void onIrVersionAccessed()
    {
        if (codecState() == CodecStates.NOT_WRAPPED)
        {
            throw new IllegalStateException("Illegal field access order. " +
                "Cannot access field \"irVersion\" in state: " + CodecStates.name(codecState()) +
                ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                "]. Please see the diagram in the Javadoc of the class FrameCodecDecoder#CodecStates.");
        }
    }

    public static int irVersionNullValue()
    {
        return -2147483648;
    }

    public static int irVersionMinValue()
    {
        return -2147483647;
    }

    public static int irVersionMaxValue()
    {
        return 2147483647;
    }

    public int irVersion()
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onIrVersionAccessed();
        }

        return buffer.getInt(offset + 4, java.nio.ByteOrder.LITTLE_ENDIAN);
    }


    public static int schemaVersionId()
    {
        return 3;
    }

    public static int schemaVersionSinceVersion()
    {
        return 0;
    }

    public static int schemaVersionEncodingOffset()
    {
        return 8;
    }

    public static int schemaVersionEncodingLength()
    {
        return 4;
    }

    public static String schemaVersionMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    private void onSchemaVersionAccessed()
    {
        if (codecState() == CodecStates.NOT_WRAPPED)
        {
            throw new IllegalStateException("Illegal field access order. " +
                "Cannot access field \"schemaVersion\" in state: " + CodecStates.name(codecState()) +
                ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                "]. Please see the diagram in the Javadoc of the class FrameCodecDecoder#CodecStates.");
        }
    }

    public static int schemaVersionNullValue()
    {
        return -2147483648;
    }

    public static int schemaVersionMinValue()
    {
        return -2147483647;
    }

    public static int schemaVersionMaxValue()
    {
        return 2147483647;
    }

    public int schemaVersion()
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onSchemaVersionAccessed();
        }

        return buffer.getInt(offset + 8, java.nio.ByteOrder.LITTLE_ENDIAN);
    }


    public static int packageNameId()
    {
        return 4;
    }

    public static int packageNameSinceVersion()
    {
        return 0;
    }

    public static String packageNameCharacterEncoding()
    {
        return java.nio.charset.StandardCharsets.UTF_8.name();
    }

    public static String packageNameMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    public static int packageNameHeaderLength()
    {
        return 2;
    }

    void onPackageNameLengthAccessed()
    {
        switch (codecState())
        {
            case CodecStates.V0_BLOCK:
                codecState(CodecStates.V0_BLOCK);
                break;
            default:
                throw new IllegalStateException("Illegal field access order. " +
                    "Cannot decode length of var data \"packageName\" in state: " + CodecStates.name(codecState()) +
                    ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                    "]. Please see the diagram in the Javadoc of the class FrameCodecDecoder#CodecStates.");
        }
    }

    private void onPackageNameAccessed()
    {
        switch (codecState())
        {
            case CodecStates.V0_BLOCK:
                codecState(CodecStates.V0_PACKAGENAME_DONE);
                break;
            default:
                throw new IllegalStateException("Illegal field access order. " +
                    "Cannot access field \"packageName\" in state: " + CodecStates.name(codecState()) +
                    ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                    "]. Please see the diagram in the Javadoc of the class FrameCodecDecoder#CodecStates.");
        }
    }

    public int packageNameLength()
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onPackageNameLengthAccessed();
        }

        final int limit = parentMessage.limit();
        return (buffer.getShort(limit, java.nio.ByteOrder.LITTLE_ENDIAN) & 0xFFFF);
    }

    public int skipPackageName()
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onPackageNameAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        final int dataLength = (buffer.getShort(limit, java.nio.ByteOrder.LITTLE_ENDIAN) & 0xFFFF);
        final int dataOffset = limit + headerLength;
        parentMessage.limit(dataOffset + dataLength);

        return dataLength;
    }

    public int getPackageName(final MutableDirectBuffer dst, final int dstOffset, final int length)
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onPackageNameAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        final int dataLength = (buffer.getShort(limit, java.nio.ByteOrder.LITTLE_ENDIAN) & 0xFFFF);
        final int bytesCopied = Math.min(length, dataLength);
        parentMessage.limit(limit + headerLength + dataLength);
        buffer.getBytes(limit + headerLength, dst, dstOffset, bytesCopied);

        return bytesCopied;
    }

    public int getPackageName(final byte[] dst, final int dstOffset, final int length)
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onPackageNameAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        final int dataLength = (buffer.getShort(limit, java.nio.ByteOrder.LITTLE_ENDIAN) & 0xFFFF);
        final int bytesCopied = Math.min(length, dataLength);
        parentMessage.limit(limit + headerLength + dataLength);
        buffer.getBytes(limit + headerLength, dst, dstOffset, bytesCopied);

        return bytesCopied;
    }

    public void wrapPackageName(final DirectBuffer wrapBuffer)
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onPackageNameAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        final int dataLength = (buffer.getShort(limit, java.nio.ByteOrder.LITTLE_ENDIAN) & 0xFFFF);
        parentMessage.limit(limit + headerLength + dataLength);
        wrapBuffer.wrap(buffer, limit + headerLength, dataLength);
    }

    public String packageName()
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onPackageNameAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        final int dataLength = (buffer.getShort(limit, java.nio.ByteOrder.LITTLE_ENDIAN) & 0xFFFF);
        parentMessage.limit(limit + headerLength + dataLength);

        if (0 == dataLength)
        {
            return "";
        }

        final byte[] tmp = new byte[dataLength];
        buffer.getBytes(limit + headerLength, tmp, 0, dataLength);

        return new String(tmp, java.nio.charset.StandardCharsets.UTF_8);
    }

    public static int namespaceNameId()
    {
        return 5;
    }

    public static int namespaceNameSinceVersion()
    {
        return 0;
    }

    public static String namespaceNameCharacterEncoding()
    {
        return java.nio.charset.StandardCharsets.UTF_8.name();
    }

    public static String namespaceNameMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    public static int namespaceNameHeaderLength()
    {
        return 2;
    }

    void onNamespaceNameLengthAccessed()
    {
        switch (codecState())
        {
            case CodecStates.V0_PACKAGENAME_DONE:
                codecState(CodecStates.V0_PACKAGENAME_DONE);
                break;
            default:
                throw new IllegalStateException("Illegal field access order. " +
                    "Cannot decode length of var data \"namespaceName\" in state: " + CodecStates.name(codecState()) +
                    ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                    "]. Please see the diagram in the Javadoc of the class FrameCodecDecoder#CodecStates.");
        }
    }

    private void onNamespaceNameAccessed()
    {
        switch (codecState())
        {
            case CodecStates.V0_PACKAGENAME_DONE:
                codecState(CodecStates.V0_NAMESPACENAME_DONE);
                break;
            default:
                throw new IllegalStateException("Illegal field access order. " +
                    "Cannot access field \"namespaceName\" in state: " + CodecStates.name(codecState()) +
                    ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                    "]. Please see the diagram in the Javadoc of the class FrameCodecDecoder#CodecStates.");
        }
    }

    public int namespaceNameLength()
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onNamespaceNameLengthAccessed();
        }

        final int limit = parentMessage.limit();
        return (buffer.getShort(limit, java.nio.ByteOrder.LITTLE_ENDIAN) & 0xFFFF);
    }

    public int skipNamespaceName()
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onNamespaceNameAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        final int dataLength = (buffer.getShort(limit, java.nio.ByteOrder.LITTLE_ENDIAN) & 0xFFFF);
        final int dataOffset = limit + headerLength;
        parentMessage.limit(dataOffset + dataLength);

        return dataLength;
    }

    public int getNamespaceName(final MutableDirectBuffer dst, final int dstOffset, final int length)
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onNamespaceNameAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        final int dataLength = (buffer.getShort(limit, java.nio.ByteOrder.LITTLE_ENDIAN) & 0xFFFF);
        final int bytesCopied = Math.min(length, dataLength);
        parentMessage.limit(limit + headerLength + dataLength);
        buffer.getBytes(limit + headerLength, dst, dstOffset, bytesCopied);

        return bytesCopied;
    }

    public int getNamespaceName(final byte[] dst, final int dstOffset, final int length)
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onNamespaceNameAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        final int dataLength = (buffer.getShort(limit, java.nio.ByteOrder.LITTLE_ENDIAN) & 0xFFFF);
        final int bytesCopied = Math.min(length, dataLength);
        parentMessage.limit(limit + headerLength + dataLength);
        buffer.getBytes(limit + headerLength, dst, dstOffset, bytesCopied);

        return bytesCopied;
    }

    public void wrapNamespaceName(final DirectBuffer wrapBuffer)
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onNamespaceNameAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        final int dataLength = (buffer.getShort(limit, java.nio.ByteOrder.LITTLE_ENDIAN) & 0xFFFF);
        parentMessage.limit(limit + headerLength + dataLength);
        wrapBuffer.wrap(buffer, limit + headerLength, dataLength);
    }

    public String namespaceName()
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onNamespaceNameAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        final int dataLength = (buffer.getShort(limit, java.nio.ByteOrder.LITTLE_ENDIAN) & 0xFFFF);
        parentMessage.limit(limit + headerLength + dataLength);

        if (0 == dataLength)
        {
            return "";
        }

        final byte[] tmp = new byte[dataLength];
        buffer.getBytes(limit + headerLength, tmp, 0, dataLength);

        return new String(tmp, java.nio.charset.StandardCharsets.UTF_8);
    }

    public static int semanticVersionId()
    {
        return 6;
    }

    public static int semanticVersionSinceVersion()
    {
        return 0;
    }

    public static String semanticVersionCharacterEncoding()
    {
        return java.nio.charset.StandardCharsets.UTF_8.name();
    }

    public static String semanticVersionMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    public static int semanticVersionHeaderLength()
    {
        return 2;
    }

    void onSemanticVersionLengthAccessed()
    {
        switch (codecState())
        {
            case CodecStates.V0_NAMESPACENAME_DONE:
                codecState(CodecStates.V0_NAMESPACENAME_DONE);
                break;
            default:
                throw new IllegalStateException("Illegal field access order. " +
                    "Cannot decode length of var data \"semanticVersion\" in state: " + CodecStates.name(codecState()) +
                    ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                    "]. Please see the diagram in the Javadoc of the class FrameCodecDecoder#CodecStates.");
        }
    }

    private void onSemanticVersionAccessed()
    {
        switch (codecState())
        {
            case CodecStates.V0_NAMESPACENAME_DONE:
                codecState(CodecStates.V0_SEMANTICVERSION_DONE);
                break;
            default:
                throw new IllegalStateException("Illegal field access order. " +
                    "Cannot access field \"semanticVersion\" in state: " + CodecStates.name(codecState()) +
                    ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                    "]. Please see the diagram in the Javadoc of the class FrameCodecDecoder#CodecStates.");
        }
    }

    public int semanticVersionLength()
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onSemanticVersionLengthAccessed();
        }

        final int limit = parentMessage.limit();
        return (buffer.getShort(limit, java.nio.ByteOrder.LITTLE_ENDIAN) & 0xFFFF);
    }

    public int skipSemanticVersion()
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onSemanticVersionAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        final int dataLength = (buffer.getShort(limit, java.nio.ByteOrder.LITTLE_ENDIAN) & 0xFFFF);
        final int dataOffset = limit + headerLength;
        parentMessage.limit(dataOffset + dataLength);

        return dataLength;
    }

    public int getSemanticVersion(final MutableDirectBuffer dst, final int dstOffset, final int length)
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onSemanticVersionAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        final int dataLength = (buffer.getShort(limit, java.nio.ByteOrder.LITTLE_ENDIAN) & 0xFFFF);
        final int bytesCopied = Math.min(length, dataLength);
        parentMessage.limit(limit + headerLength + dataLength);
        buffer.getBytes(limit + headerLength, dst, dstOffset, bytesCopied);

        return bytesCopied;
    }

    public int getSemanticVersion(final byte[] dst, final int dstOffset, final int length)
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onSemanticVersionAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        final int dataLength = (buffer.getShort(limit, java.nio.ByteOrder.LITTLE_ENDIAN) & 0xFFFF);
        final int bytesCopied = Math.min(length, dataLength);
        parentMessage.limit(limit + headerLength + dataLength);
        buffer.getBytes(limit + headerLength, dst, dstOffset, bytesCopied);

        return bytesCopied;
    }

    public void wrapSemanticVersion(final DirectBuffer wrapBuffer)
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onSemanticVersionAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        final int dataLength = (buffer.getShort(limit, java.nio.ByteOrder.LITTLE_ENDIAN) & 0xFFFF);
        parentMessage.limit(limit + headerLength + dataLength);
        wrapBuffer.wrap(buffer, limit + headerLength, dataLength);
    }

    public String semanticVersion()
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onSemanticVersionAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        final int dataLength = (buffer.getShort(limit, java.nio.ByteOrder.LITTLE_ENDIAN) & 0xFFFF);
        parentMessage.limit(limit + headerLength + dataLength);

        if (0 == dataLength)
        {
            return "";
        }

        final byte[] tmp = new byte[dataLength];
        buffer.getBytes(limit + headerLength, tmp, 0, dataLength);

        return new String(tmp, java.nio.charset.StandardCharsets.UTF_8);
    }

    public String toString()
    {
        if (null == buffer)
        {
            return "";
        }

        final FrameCodecDecoder decoder = new FrameCodecDecoder();
        decoder.wrap(buffer, offset, actingBlockLength, actingVersion);

        return decoder.appendTo(new StringBuilder()).toString();
    }

    public StringBuilder appendTo(final StringBuilder builder)
    {
        if (null == buffer)
        {
            return builder;
        }

        final int originalLimit = limit();
        limit(offset + actingBlockLength);
        builder.append("[FrameCodec](sbeTemplateId=");
        builder.append(TEMPLATE_ID);
        builder.append("|sbeSchemaId=");
        builder.append(SCHEMA_ID);
        builder.append("|sbeSchemaVersion=");
        if (parentMessage.actingVersion != SCHEMA_VERSION)
        {
            builder.append(parentMessage.actingVersion);
            builder.append('/');
        }
        builder.append(SCHEMA_VERSION);
        builder.append("|sbeBlockLength=");
        if (actingBlockLength != BLOCK_LENGTH)
        {
            builder.append(actingBlockLength);
            builder.append('/');
        }
        builder.append(BLOCK_LENGTH);
        builder.append("):");
        builder.append("irId=");
        builder.append(this.irId());
        builder.append('|');
        builder.append("irVersion=");
        builder.append(this.irVersion());
        builder.append('|');
        builder.append("schemaVersion=");
        builder.append(this.schemaVersion());
        builder.append('|');
        builder.append("packageName=");
        builder.append('\'').append(packageName()).append('\'');
        builder.append('|');
        builder.append("namespaceName=");
        builder.append('\'').append(namespaceName()).append('\'');
        builder.append('|');
        builder.append("semanticVersion=");
        builder.append('\'').append(semanticVersion()).append('\'');

        limit(originalLimit);

        return builder;
    }
    
    public FrameCodecDecoder sbeSkip()
    {
        sbeRewind();
        skipPackageName();
        skipNamespaceName();
        skipSemanticVersion();

        return this;
    }
}
