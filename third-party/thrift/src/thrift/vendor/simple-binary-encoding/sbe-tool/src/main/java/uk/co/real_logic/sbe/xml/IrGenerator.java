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
package uk.co.real_logic.sbe.xml;

import uk.co.real_logic.sbe.PrimitiveType;
import uk.co.real_logic.sbe.PrimitiveValue;
import uk.co.real_logic.sbe.ir.Encoding;
import uk.co.real_logic.sbe.ir.Ir;
import uk.co.real_logic.sbe.ir.Signal;
import uk.co.real_logic.sbe.ir.Token;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

/**
 * Class to hold the state while generating the {@link uk.co.real_logic.sbe.ir.Ir}.
 */
public class IrGenerator
{
    private final List<Token> tokenList = new ArrayList<>();
    private MessageSchema schema;

    /**
     * Generate a complete {@link uk.co.real_logic.sbe.ir.Ir} for a given schema.
     *
     * @param schema    from which the {@link uk.co.real_logic.sbe.ir.Ir} should be generated.
     * @param namespace for the generated code.
     * @return complete {@link uk.co.real_logic.sbe.ir.Ir} for a given schema.
     */
    public Ir generate(final MessageSchema schema, final String namespace)
    {
        this.schema = schema;

        final List<Token> headerTokens = generateForHeader(schema);
        final Ir ir = new Ir(
            schema.packageName(),
            namespace,
            schema.id(),
            schema.version(),
            schema.description(),
            schema.semanticVersion(),
            schema.byteOrder(),
            headerTokens);

        for (final Message message : schema.messages())
        {
            final long msgId = message.id();
            ir.addMessage(msgId, generateForMessage(schema, msgId));
        }

        return ir;
    }

    /**
     * Generate a complete {@link uk.co.real_logic.sbe.ir.Ir} for a given schema.
     *
     * @param schema from which the {@link uk.co.real_logic.sbe.ir.Ir} should be generated.
     * @return complete {@link uk.co.real_logic.sbe.ir.Ir} for a given schema.
     */
    public Ir generate(final MessageSchema schema)
    {
        return generate(schema, null);
    }

    private List<Token> generateForMessage(final MessageSchema schema, final long messageId)
    {
        tokenList.clear();

        final Message msg = schema.getMessage(messageId);

        addMessageSignal(msg, Signal.BEGIN_MESSAGE);
        addAllFields(msg.fields());
        addMessageSignal(msg, Signal.END_MESSAGE);

        return tokenList;
    }

    private List<Token> generateForHeader(final MessageSchema schema)
    {
        tokenList.clear();

        add(schema.messageHeader(), 0, null);

        return tokenList;
    }

    private void addMessageSignal(final Message msg, final Signal signal)
    {
        final Encoding encoding = new Encoding.Builder()
            .semanticType(msg.semanticType())
            .build();

        final Token token = new Token.Builder()
            .signal(signal)
            .name(msg.name())
            .description(msg.description())
            .size(msg.blockLength())
            .id(msg.id())
            .version(msg.sinceVersion())
            .deprecated(msg.deprecated())
            .encoding(encoding)
            .build();

        tokenList.add(token);
    }

    private void addFieldSignal(final Field field, final Signal signal, final int typeSinceVersion)
    {
        final Encoding.Builder encodingBuilder = new Encoding.Builder()
            .epoch(field.epoch())
            .timeUnit(field.timeUnit())
            .presence(mapPresence(field.presence()))
            .semanticType(semanticTypeOf(null, field));

        if (field.presence() == Presence.CONSTANT && null != field.valueRef())
        {
            final String valueRef = field.valueRef();
            final byte[] bytes;
            bytes = valueRef.getBytes(StandardCharsets.UTF_8);

            encodingBuilder.constValue(new PrimitiveValue(bytes, "UTF-8", valueRef.length()));
            encodingBuilder.primitiveType(PrimitiveType.CHAR);
        }

        final Token token = new Token.Builder()
            .signal(signal)
            .size(field.computedBlockLength())
            .name(field.name())
            .description(field.description())
            .id(field.id())
            .offset(field.computedOffset())
            .version(Math.max(field.sinceVersion(), typeSinceVersion))
            .deprecated(field.deprecated())
            .encoding(encodingBuilder.build())
            .build();

        tokenList.add(token);
    }

    private void addAllFields(final List<Field> fieldList)
    {
        for (final Field field : fieldList)
        {
            final Type type = field.type();

            if (null == type)
            {
                addFieldSignal(field, Signal.BEGIN_GROUP, 0);
                add(field.dimensionType(), 0, field);
                addAllFields(field.groupFields());
                addFieldSignal(field, Signal.END_GROUP, 0);
            }
            else if (type instanceof CompositeType && field.isVariableLength())
            {
                addFieldSignal(field, Signal.BEGIN_VAR_DATA, 0);
                add((CompositeType)type, field.computedOffset(), field);
                addFieldSignal(field, Signal.END_VAR_DATA, 0);
            }
            else
            {
                final int typeSinceVersion = type.sinceVersion();

                addFieldSignal(field, Signal.BEGIN_FIELD, typeSinceVersion);

                if (type instanceof EncodedDataType)
                {
                    add((EncodedDataType)type, field.computedOffset(), field);
                }
                else if (type instanceof CompositeType)
                {
                    add((CompositeType)type, field.computedOffset(), field);
                }
                else if (type instanceof EnumType)
                {
                    add((EnumType)type, field.computedOffset(), field);
                }
                else if (type instanceof SetType)
                {
                    add((SetType)type, field.computedOffset(), field);
                }
                else
                {
                    throw new IllegalStateException("Unknown type: " + type);
                }

                addFieldSignal(field, Signal.END_FIELD, typeSinceVersion);
            }
        }
    }

    private void add(final CompositeType type, final int currOffset, final Field field)
    {
        final Encoding encoding = new Encoding.Builder()
            .semanticType(semanticTypeOf(type, field))
            .build();

        final int version = null != field ? Math.max(field.sinceVersion(), type.sinceVersion()) : type.sinceVersion();

        final Token.Builder builder = new Token.Builder()
            .signal(Signal.BEGIN_COMPOSITE)
            .name(type.name())
            .packageName(type.packageName())
            .referencedName(type.referencedName())
            .offset(currOffset)
            .size(type.encodedLength())
            .version(version)
            .deprecated(type.deprecated())
            .description(type.description())
            .encoding(encoding);

        tokenList.add(builder.build());

        int offset = 0;
        for (final Type elementType : type.getTypeList())
        {
            if (elementType.offsetAttribute() != -1)
            {
                offset = elementType.offsetAttribute();
            }

            if (elementType instanceof EncodedDataType)
            {
                add((EncodedDataType)elementType,
                    offset,
                    null != field ? Math.max(field.sinceVersion(), type.sinceVersion()) : elementType.sinceVersion());
            }
            else if (elementType instanceof EnumType)
            {
                add((EnumType)elementType, offset, field);
            }
            else if (elementType instanceof SetType)
            {
                add((SetType)elementType, offset, field);
            }
            else if (elementType instanceof CompositeType)
            {
                add((CompositeType)elementType, offset, field);
            }

            offset += elementType.encodedLength();
        }

        tokenList.add(builder.signal(Signal.END_COMPOSITE).build());
    }

    private void add(final EnumType type, final int offset, final Field field)
    {
        final PrimitiveType encodingType = type.encodingType();
        final Encoding.Builder encodingBuilder = new Encoding.Builder()
            .primitiveType(encodingType)
            .semanticType(semanticTypeOf(type, field))
            .nullValue(type.nullValue())
            .byteOrder(schema.byteOrder());

        final int version = null != field ? Math.max(field.sinceVersion(), type.sinceVersion()) : type.sinceVersion();

        final Token.Builder builder = new Token.Builder()
            .signal(Signal.BEGIN_ENUM)
            .name(type.name())
            .packageName(type.packageName())
            .referencedName(type.referencedName())
            .size(encodingType.size())
            .offset(offset)
            .version(version)
            .deprecated(type.deprecated())
            .description(type.description())
            .encoding(encodingBuilder.build());

        tokenList.add(builder.build());

        for (final EnumType.ValidValue validValue : type.validValues())
        {
            add(validValue, encodingType);
        }

        builder.signal(Signal.END_ENUM);

        tokenList.add(builder.build());
    }

    private void add(final EnumType.ValidValue value, final PrimitiveType encodingType)
    {
        final Encoding encoding = new Encoding.Builder()
            .byteOrder(schema.byteOrder())
            .primitiveType(encodingType)
            .constValue(value.primitiveValue())
            .build();

        final Token.Builder builder = new Token.Builder()
            .signal(Signal.VALID_VALUE)
            .name(value.name())
            .version(value.sinceVersion())
            .deprecated(value.deprecated())
            .description(value.description())
            .encoding(encoding);

        tokenList.add(builder.build());
    }

    private void add(final SetType type, final int offset, final Field field)
    {
        final PrimitiveType encodingType = type.encodingType();

        final Encoding encoding = new Encoding.Builder()
            .semanticType(semanticTypeOf(type, field))
            .primitiveType(encodingType)
            .build();

        final int version = null != field ? Math.max(field.sinceVersion(), type.sinceVersion()) : type.sinceVersion();

        final Token.Builder builder = new Token.Builder()
            .signal(Signal.BEGIN_SET)
            .name(type.name())
            .packageName(type.packageName())
            .referencedName(type.referencedName())
            .size(encodingType.size())
            .offset(offset)
            .version(version)
            .deprecated(type.deprecated())
            .description(type.description())
            .encoding(encoding);

        tokenList.add(builder.build());

        for (final SetType.Choice choice : type.choices())
        {
            add(choice, encodingType);
        }

        builder.signal(Signal.END_SET);

        tokenList.add(builder.build());
    }

    private void add(final SetType.Choice value, final PrimitiveType encodingType)
    {
        final Encoding encoding = new Encoding.Builder()
            .constValue(value.primitiveValue())
            .byteOrder(schema.byteOrder())
            .primitiveType(encodingType)
            .build();

        final Token.Builder builder = new Token.Builder()
            .signal(Signal.CHOICE)
            .name(value.name())
            .description(value.description())
            .version(value.sinceVersion())
            .deprecated(value.deprecated())
            .encoding(encoding);

        tokenList.add(builder.build());
    }

    private void add(final EncodedDataType type, final int offset, final int sinceVersion)
    {
        final Encoding.Builder encodingBuilder = new Encoding.Builder()
            .primitiveType(type.primitiveType())
            .byteOrder(schema.byteOrder())
            .characterEncoding(type.characterEncoding());

        final Token.Builder tokenBuilder = new Token.Builder()
            .signal(Signal.ENCODING)
            .name(type.name())
            .packageName(type.packageName())
            .referencedName(type.referencedName())
            .size(type.encodedLength())
            .description(type.description())
            .version(sinceVersion)
            .deprecated(type.deprecated())
            .offset(offset);

        switch (type.presence())
        {
            case REQUIRED:
                encodingBuilder
                    .presence(Encoding.Presence.REQUIRED)
                    .minValue(type.minValue())
                    .maxValue(type.maxValue());
                break;

            case OPTIONAL:
                encodingBuilder
                    .presence(Encoding.Presence.OPTIONAL)
                    .minValue(type.minValue())
                    .maxValue(type.maxValue())
                    .nullValue(type.nullValue());
                break;

            case CONSTANT:
                tokenBuilder.size(0);
                encodingBuilder
                    .presence(Encoding.Presence.CONSTANT)
                    .constValue(type.constVal());
                break;
        }

        final Token token = tokenBuilder.encoding(encodingBuilder.build()).build();

        tokenList.add(token);
    }

    private void add(final EncodedDataType type, final int offset, final Field field)
    {
        final Encoding.Builder encodingBuilder = new Encoding.Builder()
            .primitiveType(type.primitiveType())
            .byteOrder(schema.byteOrder())
            .semanticType(semanticTypeOf(type, field))
            .characterEncoding(type.characterEncoding())
            .timeUnit(field.timeUnit())
            .epoch(field.epoch());

        final int version = Math.max(field.sinceVersion(), type.sinceVersion());

        final Token.Builder tokenBuilder = new Token.Builder()
            .signal(Signal.ENCODING)
            .name(type.name())
            .packageName(type.packageName())
            .referencedName(type.referencedName())
            .size(type.encodedLength())
            .description(type.description())
            .version(version)
            .deprecated(type.deprecated())
            .offset(offset);

        switch (field.presence())
        {
            case REQUIRED:
                encodingBuilder
                    .presence(Encoding.Presence.REQUIRED)
                    .minValue(type.minValue())
                    .maxValue(type.maxValue());
                break;

            case OPTIONAL:
                encodingBuilder
                    .presence(Encoding.Presence.OPTIONAL)
                    .minValue(type.minValue())
                    .maxValue(type.maxValue())
                    .nullValue(type.nullValue());
                break;

            case CONSTANT:
                final String valueRef = field.valueRef();
                tokenBuilder.size(0);
                encodingBuilder
                    .presence(Encoding.Presence.CONSTANT)
                    .constValue(valueRef != null ? lookupValueRef(valueRef) : type.constVal());
                break;
        }

        final Token token = tokenBuilder.encoding(encodingBuilder.build()).build();

        tokenList.add(token);
    }

    private PrimitiveValue lookupValueRef(final String valueRef)
    {
        final int periodIndex = valueRef.indexOf('.');
        final String valueRefType = valueRef.substring(0, periodIndex);
        final String validValueName = valueRef.substring(periodIndex + 1);

        final EnumType enumType = (EnumType)schema.getType(valueRefType);
        final EnumType.ValidValue validValue = enumType.getValidValue(validValueName);

        return validValue.primitiveValue();
    }

    private static String semanticTypeOf(final Type type, final Field field)
    {
        final String typeSemanticType = null != type ? type.semanticType() : null;
        if (typeSemanticType != null)
        {
            return typeSemanticType;
        }

        return null != field ? field.semanticType() : null;
    }

    private Encoding.Presence mapPresence(final Presence presence)
    {
        Encoding.Presence encodingPresence = Encoding.Presence.REQUIRED;

        if (null != presence)
        {
            switch (presence)
            {
                case OPTIONAL:
                    encodingPresence = Encoding.Presence.OPTIONAL;
                    break;

                case CONSTANT:
                    encodingPresence = Encoding.Presence.CONSTANT;
                    break;

                default:
                    break;
            }
        }

        return encodingPresence;
    }
}
