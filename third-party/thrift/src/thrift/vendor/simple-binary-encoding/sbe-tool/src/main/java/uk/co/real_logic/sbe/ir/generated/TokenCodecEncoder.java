/* Generated SBE (Simple Binary Encoding) message codec. */
package uk.co.real_logic.sbe.ir.generated;

import org.agrona.MutableDirectBuffer;
import org.agrona.DirectBuffer;


/**
 * Codec for an IR Token.
 */
@SuppressWarnings("all")
public final class TokenCodecEncoder
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
     *       V0_BLOCK -> V0_BLOCK [label="  tokenOffset(?)  "];
     *       V0_BLOCK -> V0_BLOCK [label="  tokenSize(?)  "];
     *       V0_BLOCK -> V0_BLOCK [label="  fieldId(?)  "];
     *       V0_BLOCK -> V0_BLOCK [label="  tokenVersion(?)  "];
     *       V0_BLOCK -> V0_BLOCK [label="  componentTokenCount(?)  "];
     *       V0_BLOCK -> V0_BLOCK [label="  signal(?)  "];
     *       V0_BLOCK -> V0_BLOCK [label="  primitiveType(?)  "];
     *       V0_BLOCK -> V0_BLOCK [label="  byteOrder(?)  "];
     *       V0_BLOCK -> V0_BLOCK [label="  presence(?)  "];
     *       V0_BLOCK -> V0_BLOCK [label="  deprecated(?)  "];
     *       V0_BLOCK -> V0_BLOCK [label="  nameLength()  "];
     *       V0_BLOCK -> V0_NAME_DONE [label="  name(?)  "];
     *       V0_NAME_DONE -> V0_NAME_DONE [label="  constValueLength()  "];
     *       V0_NAME_DONE -> V0_CONSTVALUE_DONE [label="  constValue(?)  "];
     *       V0_CONSTVALUE_DONE -> V0_CONSTVALUE_DONE [label="  minValueLength()  "];
     *       V0_CONSTVALUE_DONE -> V0_MINVALUE_DONE [label="  minValue(?)  "];
     *       V0_MINVALUE_DONE -> V0_MINVALUE_DONE [label="  maxValueLength()  "];
     *       V0_MINVALUE_DONE -> V0_MAXVALUE_DONE [label="  maxValue(?)  "];
     *       V0_MAXVALUE_DONE -> V0_MAXVALUE_DONE [label="  nullValueLength()  "];
     *       V0_MAXVALUE_DONE -> V0_NULLVALUE_DONE [label="  nullValue(?)  "];
     *       V0_NULLVALUE_DONE -> V0_NULLVALUE_DONE [label="  characterEncodingLength()  "];
     *       V0_NULLVALUE_DONE -> V0_CHARACTERENCODING_DONE [label="  characterEncoding(?)  "];
     *       V0_CHARACTERENCODING_DONE -> V0_CHARACTERENCODING_DONE [label="  epochLength()  "];
     *       V0_CHARACTERENCODING_DONE -> V0_EPOCH_DONE [label="  epoch(?)  "];
     *       V0_EPOCH_DONE -> V0_EPOCH_DONE [label="  timeUnitLength()  "];
     *       V0_EPOCH_DONE -> V0_TIMEUNIT_DONE [label="  timeUnit(?)  "];
     *       V0_TIMEUNIT_DONE -> V0_TIMEUNIT_DONE [label="  semanticTypeLength()  "];
     *       V0_TIMEUNIT_DONE -> V0_SEMANTICTYPE_DONE [label="  semanticType(?)  "];
     *       V0_SEMANTICTYPE_DONE -> V0_SEMANTICTYPE_DONE [label="  descriptionLength()  "];
     *       V0_SEMANTICTYPE_DONE -> V0_DESCRIPTION_DONE [label="  description(?)  "];
     *       V0_DESCRIPTION_DONE -> V0_DESCRIPTION_DONE [label="  referencedNameLength()  "];
     *       V0_DESCRIPTION_DONE -> V0_REFERENCEDNAME_DONE [label="  referencedName(?)  "];
     *   }
     * }</pre>
     */
    private static class CodecStates
    {
        private static final int NOT_WRAPPED = 0;
        private static final int V0_BLOCK = 1;
        private static final int V0_NAME_DONE = 2;
        private static final int V0_CONSTVALUE_DONE = 3;
        private static final int V0_MINVALUE_DONE = 4;
        private static final int V0_MAXVALUE_DONE = 5;
        private static final int V0_NULLVALUE_DONE = 6;
        private static final int V0_CHARACTERENCODING_DONE = 7;
        private static final int V0_EPOCH_DONE = 8;
        private static final int V0_TIMEUNIT_DONE = 9;
        private static final int V0_SEMANTICTYPE_DONE = 10;
        private static final int V0_DESCRIPTION_DONE = 11;
        private static final int V0_REFERENCEDNAME_DONE = 12;

        private static final String[] STATE_NAME_LOOKUP =
        {
            "NOT_WRAPPED",
            "V0_BLOCK",
            "V0_NAME_DONE",
            "V0_CONSTVALUE_DONE",
            "V0_MINVALUE_DONE",
            "V0_MAXVALUE_DONE",
            "V0_NULLVALUE_DONE",
            "V0_CHARACTERENCODING_DONE",
            "V0_EPOCH_DONE",
            "V0_TIMEUNIT_DONE",
            "V0_SEMANTICTYPE_DONE",
            "V0_DESCRIPTION_DONE",
            "V0_REFERENCEDNAME_DONE",
        };

        private static final String[] STATE_TRANSITIONS_LOOKUP =
        {
            "\"wrap(version=0)\"",
            "\"tokenOffset(?)\", \"tokenSize(?)\", \"fieldId(?)\", \"tokenVersion(?)\", \"componentTokenCount(?)\", \"signal(?)\", \"primitiveType(?)\", \"byteOrder(?)\", \"presence(?)\", \"deprecated(?)\", \"nameLength()\", \"name(?)\"",
            "\"constValueLength()\", \"constValue(?)\"",
            "\"minValueLength()\", \"minValue(?)\"",
            "\"maxValueLength()\", \"maxValue(?)\"",
            "\"nullValueLength()\", \"nullValue(?)\"",
            "\"characterEncodingLength()\", \"characterEncoding(?)\"",
            "\"epochLength()\", \"epoch(?)\"",
            "\"timeUnitLength()\", \"timeUnit(?)\"",
            "\"semanticTypeLength()\", \"semanticType(?)\"",
            "\"descriptionLength()\", \"description(?)\"",
            "\"referencedNameLength()\", \"referencedName(?)\"",
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

    public static final int BLOCK_LENGTH = 28;
    public static final int TEMPLATE_ID = 2;
    public static final int SCHEMA_ID = 1;
    public static final int SCHEMA_VERSION = 0;
    public static final String SEMANTIC_VERSION = "";
    public static final java.nio.ByteOrder BYTE_ORDER = java.nio.ByteOrder.LITTLE_ENDIAN;

    private final TokenCodecEncoder parentMessage = this;
    private MutableDirectBuffer buffer;
    private int offset;
    private int limit;

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

    public MutableDirectBuffer buffer()
    {
        return buffer;
    }

    public int offset()
    {
        return offset;
    }

    public TokenCodecEncoder wrap(final MutableDirectBuffer buffer, final int offset)
    {
        if (buffer != this.buffer)
        {
            this.buffer = buffer;
        }
        this.offset = offset;
        limit(offset + BLOCK_LENGTH);

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            codecState(CodecStates.V0_BLOCK);
        }

        return this;
    }

    public TokenCodecEncoder wrapAndApplyHeader(
        final MutableDirectBuffer buffer, final int offset, final MessageHeaderEncoder headerEncoder)
    {
        headerEncoder
            .wrap(buffer, offset)
            .blockLength(BLOCK_LENGTH)
            .templateId(TEMPLATE_ID)
            .schemaId(SCHEMA_ID)
            .version(SCHEMA_VERSION);

        return wrap(buffer, offset + MessageHeaderEncoder.ENCODED_LENGTH);
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

    public static int tokenOffsetId()
    {
        return 1;
    }

    public static int tokenOffsetSinceVersion()
    {
        return 0;
    }

    public static int tokenOffsetEncodingOffset()
    {
        return 0;
    }

    public static int tokenOffsetEncodingLength()
    {
        return 4;
    }

    public static String tokenOffsetMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    private void onTokenOffsetAccessed()
    {
        if (codecState() == CodecStates.NOT_WRAPPED)
        {
            throw new IllegalStateException("Illegal field access order. " +
                "Cannot access field \"tokenOffset\" in state: " + CodecStates.name(codecState()) +
                ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                "]. Please see the diagram in the Javadoc of the class TokenCodecEncoder#CodecStates.");
        }
    }

    public static int tokenOffsetNullValue()
    {
        return -2147483648;
    }

    public static int tokenOffsetMinValue()
    {
        return -2147483647;
    }

    public static int tokenOffsetMaxValue()
    {
        return 2147483647;
    }

    public TokenCodecEncoder tokenOffset(final int value)
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onTokenOffsetAccessed();
        }

        buffer.putInt(offset + 0, value, java.nio.ByteOrder.LITTLE_ENDIAN);
        return this;
    }


    public static int tokenSizeId()
    {
        return 2;
    }

    public static int tokenSizeSinceVersion()
    {
        return 0;
    }

    public static int tokenSizeEncodingOffset()
    {
        return 4;
    }

    public static int tokenSizeEncodingLength()
    {
        return 4;
    }

    public static String tokenSizeMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    private void onTokenSizeAccessed()
    {
        if (codecState() == CodecStates.NOT_WRAPPED)
        {
            throw new IllegalStateException("Illegal field access order. " +
                "Cannot access field \"tokenSize\" in state: " + CodecStates.name(codecState()) +
                ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                "]. Please see the diagram in the Javadoc of the class TokenCodecEncoder#CodecStates.");
        }
    }

    public static int tokenSizeNullValue()
    {
        return -2147483648;
    }

    public static int tokenSizeMinValue()
    {
        return -2147483647;
    }

    public static int tokenSizeMaxValue()
    {
        return 2147483647;
    }

    public TokenCodecEncoder tokenSize(final int value)
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onTokenSizeAccessed();
        }

        buffer.putInt(offset + 4, value, java.nio.ByteOrder.LITTLE_ENDIAN);
        return this;
    }


    public static int fieldIdId()
    {
        return 3;
    }

    public static int fieldIdSinceVersion()
    {
        return 0;
    }

    public static int fieldIdEncodingOffset()
    {
        return 8;
    }

    public static int fieldIdEncodingLength()
    {
        return 4;
    }

    public static String fieldIdMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    private void onFieldIdAccessed()
    {
        if (codecState() == CodecStates.NOT_WRAPPED)
        {
            throw new IllegalStateException("Illegal field access order. " +
                "Cannot access field \"fieldId\" in state: " + CodecStates.name(codecState()) +
                ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                "]. Please see the diagram in the Javadoc of the class TokenCodecEncoder#CodecStates.");
        }
    }

    public static int fieldIdNullValue()
    {
        return -2147483648;
    }

    public static int fieldIdMinValue()
    {
        return -2147483647;
    }

    public static int fieldIdMaxValue()
    {
        return 2147483647;
    }

    public TokenCodecEncoder fieldId(final int value)
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onFieldIdAccessed();
        }

        buffer.putInt(offset + 8, value, java.nio.ByteOrder.LITTLE_ENDIAN);
        return this;
    }


    public static int tokenVersionId()
    {
        return 4;
    }

    public static int tokenVersionSinceVersion()
    {
        return 0;
    }

    public static int tokenVersionEncodingOffset()
    {
        return 12;
    }

    public static int tokenVersionEncodingLength()
    {
        return 4;
    }

    public static String tokenVersionMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    private void onTokenVersionAccessed()
    {
        if (codecState() == CodecStates.NOT_WRAPPED)
        {
            throw new IllegalStateException("Illegal field access order. " +
                "Cannot access field \"tokenVersion\" in state: " + CodecStates.name(codecState()) +
                ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                "]. Please see the diagram in the Javadoc of the class TokenCodecEncoder#CodecStates.");
        }
    }

    public static int tokenVersionNullValue()
    {
        return -2147483648;
    }

    public static int tokenVersionMinValue()
    {
        return -2147483647;
    }

    public static int tokenVersionMaxValue()
    {
        return 2147483647;
    }

    public TokenCodecEncoder tokenVersion(final int value)
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onTokenVersionAccessed();
        }

        buffer.putInt(offset + 12, value, java.nio.ByteOrder.LITTLE_ENDIAN);
        return this;
    }


    public static int componentTokenCountId()
    {
        return 5;
    }

    public static int componentTokenCountSinceVersion()
    {
        return 0;
    }

    public static int componentTokenCountEncodingOffset()
    {
        return 16;
    }

    public static int componentTokenCountEncodingLength()
    {
        return 4;
    }

    public static String componentTokenCountMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    private void onComponentTokenCountAccessed()
    {
        if (codecState() == CodecStates.NOT_WRAPPED)
        {
            throw new IllegalStateException("Illegal field access order. " +
                "Cannot access field \"componentTokenCount\" in state: " + CodecStates.name(codecState()) +
                ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                "]. Please see the diagram in the Javadoc of the class TokenCodecEncoder#CodecStates.");
        }
    }

    public static int componentTokenCountNullValue()
    {
        return -2147483648;
    }

    public static int componentTokenCountMinValue()
    {
        return -2147483647;
    }

    public static int componentTokenCountMaxValue()
    {
        return 2147483647;
    }

    public TokenCodecEncoder componentTokenCount(final int value)
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onComponentTokenCountAccessed();
        }

        buffer.putInt(offset + 16, value, java.nio.ByteOrder.LITTLE_ENDIAN);
        return this;
    }


    public static int signalId()
    {
        return 6;
    }

    public static int signalSinceVersion()
    {
        return 0;
    }

    public static int signalEncodingOffset()
    {
        return 20;
    }

    public static int signalEncodingLength()
    {
        return 1;
    }

    public static String signalMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    private void onSignalAccessed()
    {
        if (codecState() == CodecStates.NOT_WRAPPED)
        {
            throw new IllegalStateException("Illegal field access order. " +
                "Cannot access field \"signal\" in state: " + CodecStates.name(codecState()) +
                ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                "]. Please see the diagram in the Javadoc of the class TokenCodecEncoder#CodecStates.");
        }
    }

    public TokenCodecEncoder signal(final SignalCodec value)
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onSignalAccessed();
        }

        buffer.putByte(offset + 20, (byte)value.value());
        return this;
    }

    public static int primitiveTypeId()
    {
        return 7;
    }

    public static int primitiveTypeSinceVersion()
    {
        return 0;
    }

    public static int primitiveTypeEncodingOffset()
    {
        return 21;
    }

    public static int primitiveTypeEncodingLength()
    {
        return 1;
    }

    public static String primitiveTypeMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    private void onPrimitiveTypeAccessed()
    {
        if (codecState() == CodecStates.NOT_WRAPPED)
        {
            throw new IllegalStateException("Illegal field access order. " +
                "Cannot access field \"primitiveType\" in state: " + CodecStates.name(codecState()) +
                ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                "]. Please see the diagram in the Javadoc of the class TokenCodecEncoder#CodecStates.");
        }
    }

    public TokenCodecEncoder primitiveType(final PrimitiveTypeCodec value)
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onPrimitiveTypeAccessed();
        }

        buffer.putByte(offset + 21, (byte)value.value());
        return this;
    }

    public static int byteOrderId()
    {
        return 8;
    }

    public static int byteOrderSinceVersion()
    {
        return 0;
    }

    public static int byteOrderEncodingOffset()
    {
        return 22;
    }

    public static int byteOrderEncodingLength()
    {
        return 1;
    }

    public static String byteOrderMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    private void onByteOrderAccessed()
    {
        if (codecState() == CodecStates.NOT_WRAPPED)
        {
            throw new IllegalStateException("Illegal field access order. " +
                "Cannot access field \"byteOrder\" in state: " + CodecStates.name(codecState()) +
                ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                "]. Please see the diagram in the Javadoc of the class TokenCodecEncoder#CodecStates.");
        }
    }

    public TokenCodecEncoder byteOrder(final ByteOrderCodec value)
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onByteOrderAccessed();
        }

        buffer.putByte(offset + 22, (byte)value.value());
        return this;
    }

    public static int presenceId()
    {
        return 9;
    }

    public static int presenceSinceVersion()
    {
        return 0;
    }

    public static int presenceEncodingOffset()
    {
        return 23;
    }

    public static int presenceEncodingLength()
    {
        return 1;
    }

    public static String presenceMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    private void onPresenceAccessed()
    {
        if (codecState() == CodecStates.NOT_WRAPPED)
        {
            throw new IllegalStateException("Illegal field access order. " +
                "Cannot access field \"presence\" in state: " + CodecStates.name(codecState()) +
                ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                "]. Please see the diagram in the Javadoc of the class TokenCodecEncoder#CodecStates.");
        }
    }

    public TokenCodecEncoder presence(final PresenceCodec value)
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onPresenceAccessed();
        }

        buffer.putByte(offset + 23, (byte)value.value());
        return this;
    }

    public static int deprecatedId()
    {
        return 10;
    }

    public static int deprecatedSinceVersion()
    {
        return 0;
    }

    public static int deprecatedEncodingOffset()
    {
        return 24;
    }

    public static int deprecatedEncodingLength()
    {
        return 4;
    }

    public static String deprecatedMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "optional";
        }

        return "";
    }

    private void onDeprecatedAccessed()
    {
        if (codecState() == CodecStates.NOT_WRAPPED)
        {
            throw new IllegalStateException("Illegal field access order. " +
                "Cannot access field \"deprecated\" in state: " + CodecStates.name(codecState()) +
                ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                "]. Please see the diagram in the Javadoc of the class TokenCodecEncoder#CodecStates.");
        }
    }

    public static int deprecatedNullValue()
    {
        return 0;
    }

    public static int deprecatedMinValue()
    {
        return -2147483647;
    }

    public static int deprecatedMaxValue()
    {
        return 2147483647;
    }

    public TokenCodecEncoder deprecated(final int value)
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onDeprecatedAccessed();
        }

        buffer.putInt(offset + 24, value, java.nio.ByteOrder.LITTLE_ENDIAN);
        return this;
    }


    public static int nameId()
    {
        return 11;
    }

    public static String nameCharacterEncoding()
    {
        return java.nio.charset.StandardCharsets.UTF_8.name();
    }

    public static String nameMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    public static int nameHeaderLength()
    {
        return 2;
    }

    private void onNameAccessed()
    {
        switch (codecState())
        {
            case CodecStates.V0_BLOCK:
                codecState(CodecStates.V0_NAME_DONE);
                break;
            default:
                throw new IllegalStateException("Illegal field access order. " +
                    "Cannot access field \"name\" in state: " + CodecStates.name(codecState()) +
                    ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                    "]. Please see the diagram in the Javadoc of the class TokenCodecEncoder#CodecStates.");
        }
    }

    public TokenCodecEncoder putName(final DirectBuffer src, final int srcOffset, final int length)
    {
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onNameAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, src, srcOffset, length);

        return this;
    }

    public TokenCodecEncoder putName(final byte[] src, final int srcOffset, final int length)
    {
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onNameAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, src, srcOffset, length);

        return this;
    }

    public TokenCodecEncoder name(final String value)
    {
        final byte[] bytes = (null == value || value.isEmpty()) ? org.agrona.collections.ArrayUtil.EMPTY_BYTE_ARRAY : value.getBytes(java.nio.charset.StandardCharsets.UTF_8);

        final int length = bytes.length;
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onNameAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, bytes, 0, length);

        return this;
    }

    public static int constValueId()
    {
        return 12;
    }

    public static String constValueCharacterEncoding()
    {
        return java.nio.charset.StandardCharsets.UTF_8.name();
    }

    public static String constValueMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    public static int constValueHeaderLength()
    {
        return 2;
    }

    private void onConstValueAccessed()
    {
        switch (codecState())
        {
            case CodecStates.V0_NAME_DONE:
                codecState(CodecStates.V0_CONSTVALUE_DONE);
                break;
            default:
                throw new IllegalStateException("Illegal field access order. " +
                    "Cannot access field \"constValue\" in state: " + CodecStates.name(codecState()) +
                    ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                    "]. Please see the diagram in the Javadoc of the class TokenCodecEncoder#CodecStates.");
        }
    }

    public TokenCodecEncoder putConstValue(final DirectBuffer src, final int srcOffset, final int length)
    {
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onConstValueAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, src, srcOffset, length);

        return this;
    }

    public TokenCodecEncoder putConstValue(final byte[] src, final int srcOffset, final int length)
    {
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onConstValueAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, src, srcOffset, length);

        return this;
    }

    public TokenCodecEncoder constValue(final String value)
    {
        final byte[] bytes = (null == value || value.isEmpty()) ? org.agrona.collections.ArrayUtil.EMPTY_BYTE_ARRAY : value.getBytes(java.nio.charset.StandardCharsets.UTF_8);

        final int length = bytes.length;
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onConstValueAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, bytes, 0, length);

        return this;
    }

    public static int minValueId()
    {
        return 13;
    }

    public static String minValueCharacterEncoding()
    {
        return java.nio.charset.StandardCharsets.UTF_8.name();
    }

    public static String minValueMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    public static int minValueHeaderLength()
    {
        return 2;
    }

    private void onMinValueAccessed()
    {
        switch (codecState())
        {
            case CodecStates.V0_CONSTVALUE_DONE:
                codecState(CodecStates.V0_MINVALUE_DONE);
                break;
            default:
                throw new IllegalStateException("Illegal field access order. " +
                    "Cannot access field \"minValue\" in state: " + CodecStates.name(codecState()) +
                    ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                    "]. Please see the diagram in the Javadoc of the class TokenCodecEncoder#CodecStates.");
        }
    }

    public TokenCodecEncoder putMinValue(final DirectBuffer src, final int srcOffset, final int length)
    {
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onMinValueAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, src, srcOffset, length);

        return this;
    }

    public TokenCodecEncoder putMinValue(final byte[] src, final int srcOffset, final int length)
    {
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onMinValueAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, src, srcOffset, length);

        return this;
    }

    public TokenCodecEncoder minValue(final String value)
    {
        final byte[] bytes = (null == value || value.isEmpty()) ? org.agrona.collections.ArrayUtil.EMPTY_BYTE_ARRAY : value.getBytes(java.nio.charset.StandardCharsets.UTF_8);

        final int length = bytes.length;
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onMinValueAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, bytes, 0, length);

        return this;
    }

    public static int maxValueId()
    {
        return 14;
    }

    public static String maxValueCharacterEncoding()
    {
        return java.nio.charset.StandardCharsets.UTF_8.name();
    }

    public static String maxValueMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    public static int maxValueHeaderLength()
    {
        return 2;
    }

    private void onMaxValueAccessed()
    {
        switch (codecState())
        {
            case CodecStates.V0_MINVALUE_DONE:
                codecState(CodecStates.V0_MAXVALUE_DONE);
                break;
            default:
                throw new IllegalStateException("Illegal field access order. " +
                    "Cannot access field \"maxValue\" in state: " + CodecStates.name(codecState()) +
                    ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                    "]. Please see the diagram in the Javadoc of the class TokenCodecEncoder#CodecStates.");
        }
    }

    public TokenCodecEncoder putMaxValue(final DirectBuffer src, final int srcOffset, final int length)
    {
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onMaxValueAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, src, srcOffset, length);

        return this;
    }

    public TokenCodecEncoder putMaxValue(final byte[] src, final int srcOffset, final int length)
    {
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onMaxValueAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, src, srcOffset, length);

        return this;
    }

    public TokenCodecEncoder maxValue(final String value)
    {
        final byte[] bytes = (null == value || value.isEmpty()) ? org.agrona.collections.ArrayUtil.EMPTY_BYTE_ARRAY : value.getBytes(java.nio.charset.StandardCharsets.UTF_8);

        final int length = bytes.length;
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onMaxValueAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, bytes, 0, length);

        return this;
    }

    public static int nullValueId()
    {
        return 15;
    }

    public static String nullValueCharacterEncoding()
    {
        return java.nio.charset.StandardCharsets.UTF_8.name();
    }

    public static String nullValueMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    public static int nullValueHeaderLength()
    {
        return 2;
    }

    private void onNullValueAccessed()
    {
        switch (codecState())
        {
            case CodecStates.V0_MAXVALUE_DONE:
                codecState(CodecStates.V0_NULLVALUE_DONE);
                break;
            default:
                throw new IllegalStateException("Illegal field access order. " +
                    "Cannot access field \"nullValue\" in state: " + CodecStates.name(codecState()) +
                    ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                    "]. Please see the diagram in the Javadoc of the class TokenCodecEncoder#CodecStates.");
        }
    }

    public TokenCodecEncoder putNullValue(final DirectBuffer src, final int srcOffset, final int length)
    {
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onNullValueAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, src, srcOffset, length);

        return this;
    }

    public TokenCodecEncoder putNullValue(final byte[] src, final int srcOffset, final int length)
    {
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onNullValueAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, src, srcOffset, length);

        return this;
    }

    public TokenCodecEncoder nullValue(final String value)
    {
        final byte[] bytes = (null == value || value.isEmpty()) ? org.agrona.collections.ArrayUtil.EMPTY_BYTE_ARRAY : value.getBytes(java.nio.charset.StandardCharsets.UTF_8);

        final int length = bytes.length;
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onNullValueAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, bytes, 0, length);

        return this;
    }

    public static int characterEncodingId()
    {
        return 16;
    }

    public static String characterEncodingCharacterEncoding()
    {
        return java.nio.charset.StandardCharsets.UTF_8.name();
    }

    public static String characterEncodingMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    public static int characterEncodingHeaderLength()
    {
        return 2;
    }

    private void onCharacterEncodingAccessed()
    {
        switch (codecState())
        {
            case CodecStates.V0_NULLVALUE_DONE:
                codecState(CodecStates.V0_CHARACTERENCODING_DONE);
                break;
            default:
                throw new IllegalStateException("Illegal field access order. " +
                    "Cannot access field \"characterEncoding\" in state: " + CodecStates.name(codecState()) +
                    ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                    "]. Please see the diagram in the Javadoc of the class TokenCodecEncoder#CodecStates.");
        }
    }

    public TokenCodecEncoder putCharacterEncoding(final DirectBuffer src, final int srcOffset, final int length)
    {
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onCharacterEncodingAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, src, srcOffset, length);

        return this;
    }

    public TokenCodecEncoder putCharacterEncoding(final byte[] src, final int srcOffset, final int length)
    {
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onCharacterEncodingAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, src, srcOffset, length);

        return this;
    }

    public TokenCodecEncoder characterEncoding(final String value)
    {
        final byte[] bytes = (null == value || value.isEmpty()) ? org.agrona.collections.ArrayUtil.EMPTY_BYTE_ARRAY : value.getBytes(java.nio.charset.StandardCharsets.UTF_8);

        final int length = bytes.length;
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onCharacterEncodingAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, bytes, 0, length);

        return this;
    }

    public static int epochId()
    {
        return 17;
    }

    public static String epochCharacterEncoding()
    {
        return java.nio.charset.StandardCharsets.UTF_8.name();
    }

    public static String epochMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    public static int epochHeaderLength()
    {
        return 2;
    }

    private void onEpochAccessed()
    {
        switch (codecState())
        {
            case CodecStates.V0_CHARACTERENCODING_DONE:
                codecState(CodecStates.V0_EPOCH_DONE);
                break;
            default:
                throw new IllegalStateException("Illegal field access order. " +
                    "Cannot access field \"epoch\" in state: " + CodecStates.name(codecState()) +
                    ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                    "]. Please see the diagram in the Javadoc of the class TokenCodecEncoder#CodecStates.");
        }
    }

    public TokenCodecEncoder putEpoch(final DirectBuffer src, final int srcOffset, final int length)
    {
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onEpochAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, src, srcOffset, length);

        return this;
    }

    public TokenCodecEncoder putEpoch(final byte[] src, final int srcOffset, final int length)
    {
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onEpochAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, src, srcOffset, length);

        return this;
    }

    public TokenCodecEncoder epoch(final String value)
    {
        final byte[] bytes = (null == value || value.isEmpty()) ? org.agrona.collections.ArrayUtil.EMPTY_BYTE_ARRAY : value.getBytes(java.nio.charset.StandardCharsets.UTF_8);

        final int length = bytes.length;
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onEpochAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, bytes, 0, length);

        return this;
    }

    public static int timeUnitId()
    {
        return 18;
    }

    public static String timeUnitCharacterEncoding()
    {
        return java.nio.charset.StandardCharsets.UTF_8.name();
    }

    public static String timeUnitMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    public static int timeUnitHeaderLength()
    {
        return 2;
    }

    private void onTimeUnitAccessed()
    {
        switch (codecState())
        {
            case CodecStates.V0_EPOCH_DONE:
                codecState(CodecStates.V0_TIMEUNIT_DONE);
                break;
            default:
                throw new IllegalStateException("Illegal field access order. " +
                    "Cannot access field \"timeUnit\" in state: " + CodecStates.name(codecState()) +
                    ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                    "]. Please see the diagram in the Javadoc of the class TokenCodecEncoder#CodecStates.");
        }
    }

    public TokenCodecEncoder putTimeUnit(final DirectBuffer src, final int srcOffset, final int length)
    {
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onTimeUnitAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, src, srcOffset, length);

        return this;
    }

    public TokenCodecEncoder putTimeUnit(final byte[] src, final int srcOffset, final int length)
    {
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onTimeUnitAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, src, srcOffset, length);

        return this;
    }

    public TokenCodecEncoder timeUnit(final String value)
    {
        final byte[] bytes = (null == value || value.isEmpty()) ? org.agrona.collections.ArrayUtil.EMPTY_BYTE_ARRAY : value.getBytes(java.nio.charset.StandardCharsets.UTF_8);

        final int length = bytes.length;
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onTimeUnitAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, bytes, 0, length);

        return this;
    }

    public static int semanticTypeId()
    {
        return 19;
    }

    public static String semanticTypeCharacterEncoding()
    {
        return java.nio.charset.StandardCharsets.UTF_8.name();
    }

    public static String semanticTypeMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    public static int semanticTypeHeaderLength()
    {
        return 2;
    }

    private void onSemanticTypeAccessed()
    {
        switch (codecState())
        {
            case CodecStates.V0_TIMEUNIT_DONE:
                codecState(CodecStates.V0_SEMANTICTYPE_DONE);
                break;
            default:
                throw new IllegalStateException("Illegal field access order. " +
                    "Cannot access field \"semanticType\" in state: " + CodecStates.name(codecState()) +
                    ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                    "]. Please see the diagram in the Javadoc of the class TokenCodecEncoder#CodecStates.");
        }
    }

    public TokenCodecEncoder putSemanticType(final DirectBuffer src, final int srcOffset, final int length)
    {
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onSemanticTypeAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, src, srcOffset, length);

        return this;
    }

    public TokenCodecEncoder putSemanticType(final byte[] src, final int srcOffset, final int length)
    {
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onSemanticTypeAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, src, srcOffset, length);

        return this;
    }

    public TokenCodecEncoder semanticType(final String value)
    {
        final byte[] bytes = (null == value || value.isEmpty()) ? org.agrona.collections.ArrayUtil.EMPTY_BYTE_ARRAY : value.getBytes(java.nio.charset.StandardCharsets.UTF_8);

        final int length = bytes.length;
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onSemanticTypeAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, bytes, 0, length);

        return this;
    }

    public static int descriptionId()
    {
        return 20;
    }

    public static String descriptionCharacterEncoding()
    {
        return java.nio.charset.StandardCharsets.UTF_8.name();
    }

    public static String descriptionMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    public static int descriptionHeaderLength()
    {
        return 2;
    }

    private void onDescriptionAccessed()
    {
        switch (codecState())
        {
            case CodecStates.V0_SEMANTICTYPE_DONE:
                codecState(CodecStates.V0_DESCRIPTION_DONE);
                break;
            default:
                throw new IllegalStateException("Illegal field access order. " +
                    "Cannot access field \"description\" in state: " + CodecStates.name(codecState()) +
                    ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                    "]. Please see the diagram in the Javadoc of the class TokenCodecEncoder#CodecStates.");
        }
    }

    public TokenCodecEncoder putDescription(final DirectBuffer src, final int srcOffset, final int length)
    {
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onDescriptionAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, src, srcOffset, length);

        return this;
    }

    public TokenCodecEncoder putDescription(final byte[] src, final int srcOffset, final int length)
    {
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onDescriptionAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, src, srcOffset, length);

        return this;
    }

    public TokenCodecEncoder description(final String value)
    {
        final byte[] bytes = (null == value || value.isEmpty()) ? org.agrona.collections.ArrayUtil.EMPTY_BYTE_ARRAY : value.getBytes(java.nio.charset.StandardCharsets.UTF_8);

        final int length = bytes.length;
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onDescriptionAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, bytes, 0, length);

        return this;
    }

    public static int referencedNameId()
    {
        return 21;
    }

    public static String referencedNameCharacterEncoding()
    {
        return java.nio.charset.StandardCharsets.UTF_8.name();
    }

    public static String referencedNameMetaAttribute(final MetaAttribute metaAttribute)
    {
        if (MetaAttribute.PRESENCE == metaAttribute)
        {
            return "required";
        }

        return "";
    }

    public static int referencedNameHeaderLength()
    {
        return 2;
    }

    private void onReferencedNameAccessed()
    {
        switch (codecState())
        {
            case CodecStates.V0_DESCRIPTION_DONE:
                codecState(CodecStates.V0_REFERENCEDNAME_DONE);
                break;
            default:
                throw new IllegalStateException("Illegal field access order. " +
                    "Cannot access field \"referencedName\" in state: " + CodecStates.name(codecState()) +
                    ". Expected one of these transitions: [" + CodecStates.transitions(codecState()) +
                    "]. Please see the diagram in the Javadoc of the class TokenCodecEncoder#CodecStates.");
        }
    }

    public TokenCodecEncoder putReferencedName(final DirectBuffer src, final int srcOffset, final int length)
    {
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onReferencedNameAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, src, srcOffset, length);

        return this;
    }

    public TokenCodecEncoder putReferencedName(final byte[] src, final int srcOffset, final int length)
    {
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onReferencedNameAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, src, srcOffset, length);

        return this;
    }

    public TokenCodecEncoder referencedName(final String value)
    {
        final byte[] bytes = (null == value || value.isEmpty()) ? org.agrona.collections.ArrayUtil.EMPTY_BYTE_ARRAY : value.getBytes(java.nio.charset.StandardCharsets.UTF_8);

        final int length = bytes.length;
        if (length > 65534)
        {
            throw new IllegalStateException("length > maxValue for type: " + length);
        }

        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            onReferencedNameAccessed();
        }

        final int headerLength = 2;
        final int limit = parentMessage.limit();
        parentMessage.limit(limit + headerLength + length);
        buffer.putShort(limit, (short)length, java.nio.ByteOrder.LITTLE_ENDIAN);
        buffer.putBytes(limit + headerLength, bytes, 0, length);

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

        final TokenCodecDecoder decoder = new TokenCodecDecoder();
        decoder.wrap(buffer, offset, BLOCK_LENGTH, SCHEMA_VERSION);

        return decoder.appendTo(builder);
    }

    public void checkEncodingIsComplete()
    {
        if (SBE_ENABLE_IR_PRECEDENCE_CHECKS)
        {
            switch (codecState)
            {
                case CodecStates.V0_REFERENCEDNAME_DONE:
                    return;
                default:
                    throw new IllegalStateException("Not fully encoded, current state: " +
                        CodecStates.name(codecState) + ", allowed transitions: " +
                        CodecStates.transitions(codecState));
            }
        }
    }

}
