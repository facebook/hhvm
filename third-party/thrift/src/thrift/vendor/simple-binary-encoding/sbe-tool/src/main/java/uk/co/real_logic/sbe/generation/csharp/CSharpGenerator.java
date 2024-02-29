/*
 * Copyright 2013-2024 Real Logic Limited.
 * Copyright (C) 2017 MarketFactory, Inc
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
package uk.co.real_logic.sbe.generation.csharp;

import uk.co.real_logic.sbe.PrimitiveType;
import uk.co.real_logic.sbe.PrimitiveValue;
import uk.co.real_logic.sbe.generation.CodeGenerator;
import uk.co.real_logic.sbe.generation.Generators;
import uk.co.real_logic.sbe.generation.common.FieldPrecedenceModel;
import uk.co.real_logic.sbe.generation.common.PrecedenceChecks;
import uk.co.real_logic.sbe.ir.Encoding;
import uk.co.real_logic.sbe.ir.Ir;
import uk.co.real_logic.sbe.ir.Signal;
import uk.co.real_logic.sbe.ir.Token;
import org.agrona.Verify;
import org.agrona.collections.MutableBoolean;
import org.agrona.generation.OutputManager;

import java.io.IOException;
import java.io.Writer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import static java.lang.System.lineSeparator;
import static uk.co.real_logic.sbe.generation.Generators.toLowerFirstChar;
import static uk.co.real_logic.sbe.generation.Generators.toUpperFirstChar;
import static uk.co.real_logic.sbe.generation.csharp.CSharpUtil.*;
import static uk.co.real_logic.sbe.ir.GenerationUtil.*;

/**
 * Codec generator for the CSharp programming language.
 */
@SuppressWarnings("MethodLength")
public class CSharpGenerator implements CodeGenerator
{
    private static final String META_ATTRIBUTE_ENUM = "MetaAttribute";
    private static final String INDENT = "    ";
    private static final String TWO_INDENT = INDENT + INDENT;
    private static final String THREE_INDENT = INDENT + INDENT + INDENT;
    private static final String BASE_INDENT = INDENT;

    private final Ir ir;
    private final OutputManager outputManager;
    private final PrecedenceChecks precedenceChecks;
    private final String precedenceChecksFlagName;

    /**
     * Create a new C# language {@link CodeGenerator}.
     *
     * @param ir            for the messages and types.
     * @param outputManager for generating the codecs to.
     */
    public CSharpGenerator(final Ir ir, final OutputManager outputManager)
    {
        this(
            ir,
            PrecedenceChecks.newInstance(new PrecedenceChecks.Context()),
            outputManager
        );
    }

    /**
     * Create a new C# language {@link CodeGenerator}.
     *
     * @param ir               for the messages and types.
     * @param precedenceChecks whether and how to perform field precedence checks.
     * @param outputManager    for generating the codecs to.
     */
    public CSharpGenerator(
        final Ir ir,
        final PrecedenceChecks precedenceChecks,
        final OutputManager outputManager)
    {
        Verify.notNull(ir, "ir");
        Verify.notNull(outputManager, "outputManager");

        this.ir = ir;
        this.precedenceChecks = precedenceChecks;
        this.precedenceChecksFlagName = precedenceChecks.context().precedenceChecksFlagName();
        this.outputManager = outputManager;
    }

    /**
     * Generate the composites for dealing with the message header.
     *
     * @throws IOException if an error is encountered when writing the output.
     */
    public void generateMessageHeaderStub() throws IOException
    {
        generateComposite(ir.headerStructure().tokens());
    }

    /**
     * Generate the stubs for the types used as message fields.
     *
     * @throws IOException if an error is encountered when writing the output.
     */
    public void generateTypeStubs() throws IOException
    {
        generateMetaAttributeEnum();

        for (final List<Token> tokens : ir.types())
        {
            switch (tokens.get(0).signal())
            {
                case BEGIN_ENUM:
                    generateEnum(tokens);
                    break;

                case BEGIN_SET:
                    generateBitSet(tokens);
                    break;

                case BEGIN_COMPOSITE:
                    generateComposite(tokens);
                    break;

                default:
                    break;
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    public void generate() throws IOException
    {
        generateMessageHeaderStub();
        generateTypeStubs();

        for (final List<Token> tokens : ir.messages())
        {
            final Token msgToken = tokens.get(0);
            final String className = formatClassName(msgToken.name());
            final String stateClassName = className + ".CodecState";
            final FieldPrecedenceModel fieldPrecedenceModel = precedenceChecks.createCodecModel(stateClassName, tokens);

            try (Writer out = outputManager.createOutput(className))
            {
                final List<Token> messageBody = tokens.subList(1, tokens.size() - 1);
                int offset = 0;
                final List<Token> fields = new ArrayList<>();
                offset = collectFields(messageBody, offset, fields);
                final List<Token> groups = new ArrayList<>();
                offset = collectGroups(messageBody, offset, groups);
                final List<Token> varData = new ArrayList<>();
                collectVarData(messageBody, offset, varData);

                out.append(generateFileHeader(ir.applicableNamespace()));
                out.append(generateDocumentation(BASE_INDENT, msgToken));
                out.append(generateClassDeclaration(className));
                out.append(generateMessageFlyweightCode(className, msgToken, fieldPrecedenceModel, BASE_INDENT));

                out.append(generateFieldOrderStates(BASE_INDENT + INDENT, fieldPrecedenceModel));
                out.append(generateFullyEncodedCheck(BASE_INDENT + INDENT, fieldPrecedenceModel));

                out.append(generateFields(fieldPrecedenceModel, fields, BASE_INDENT));

                final StringBuilder sb = new StringBuilder();
                generateGroups(sb, className, groups, fieldPrecedenceModel, BASE_INDENT);
                out.append(sb);

                out.append(generateVarData(fieldPrecedenceModel, varData, BASE_INDENT + INDENT));

                out.append(generateDisplay(toUpperFirstChar(msgToken.name()),
                    fields, groups, varData, fieldPrecedenceModel));

                out.append(INDENT + "}\n");
                out.append("}\n");
            }
        }
    }

    private void generateGroups(
        final StringBuilder sb,
        final String parentMessageClassName,
        final List<Token> tokens,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent)
    {
        for (int i = 0, size = tokens.size(); i < size; i++)
        {
            final Token groupToken = tokens.get(i);
            if (groupToken.signal() != Signal.BEGIN_GROUP)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_GROUP: token=" + groupToken);
            }
            final String groupName = groupToken.name();
            sb.append(generateGroupProperty(groupName, fieldPrecedenceModel, groupToken, indent + INDENT));

            generateGroupClassHeader(sb, groupName, parentMessageClassName, tokens,
                fieldPrecedenceModel, i, indent + INDENT);
            i++;
            i += tokens.get(i).componentTokenCount();

            final List<Token> fields = new ArrayList<>();
            i = collectFields(tokens, i, fields);
            sb.append(generateFields(fieldPrecedenceModel, fields, indent + INDENT));

            final List<Token> groups = new ArrayList<>();
            i = collectGroups(tokens, i, groups);
            generateGroups(sb, parentMessageClassName, groups, fieldPrecedenceModel, indent + INDENT);

            final List<Token> varData = new ArrayList<>();
            i = collectVarData(tokens, i, varData);
            sb.append(generateVarData(fieldPrecedenceModel, varData, indent + INDENT + INDENT));

            appendGroupInstanceDisplay(sb, fields, groups, varData, indent + TWO_INDENT);

            sb.append(indent).append(INDENT + "}\n");
        }
    }

    private void generateGroupClassHeader(
        final StringBuilder sb,
        final String groupName,
        final String parentMessageClassName,
        final List<Token> tokens,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final int index,
        final String indent)
    {
        final String dimensionsClassName = formatClassName(tokens.get(index + 1).name());
        final int dimensionHeaderLength = tokens.get(index + 1).encodedLength();

        sb.append(String.format("\n" +
            "%1$s" +
            indent + "public sealed partial class %2$sGroup\n" +
            indent + "{\n" +
            indent + INDENT + "private readonly %3$s _dimensions = new %3$s();\n" +
            indent + INDENT + "private %4$s _parentMessage;\n" +
            indent + INDENT + "private DirectBuffer _buffer;\n" +
            indent + INDENT + "private int _blockLength;\n" +
            indent + INDENT + "private int _actingVersion;\n" +
            indent + INDENT + "private int _count;\n" +
            indent + INDENT + "private int _index;\n" +
            indent + INDENT + "private int _offset;\n",
            generateDocumentation(indent, tokens.get(index)),
            formatClassName(groupName),
            dimensionsClassName,
            parentMessageClassName));

        final Token numInGroupToken = Generators.findFirst("numInGroup", tokens, index);
        final boolean isIntCastSafe = isRepresentableByInt32(numInGroupToken.encoding());

        if (!isIntCastSafe)
        {
            throw new IllegalArgumentException(String.format(
                "%s.numInGroup - cannot be represented safely by an int. Please constrain the maxValue.",
                groupName));
        }

        sb.append("\n")
            // The "file" access modifier is more suitable but only available from C# 11 onwards.
            .append(indent).append(INDENT).append("internal void NotPresent()\n")
            .append(indent).append(INDENT).append("{\n")
            .append(indent).append(TWO_INDENT).append("_count = 0;\n")
            .append(indent).append(TWO_INDENT).append("_index = 0;\n")
            .append(indent).append(TWO_INDENT).append("_buffer = null;\n")
            .append(indent).append(TWO_INDENT).append("_offset = 0;\n")
            .append(indent).append(INDENT).append("}\n");

        sb.append(String.format("\n" +
            indent + INDENT + "public void WrapForDecode(%s parentMessage, DirectBuffer buffer, int actingVersion)\n" +
            indent + INDENT + "{\n" +
            indent + INDENT + INDENT + "_parentMessage = parentMessage;\n" +
            indent + INDENT + INDENT + "_buffer = buffer;\n" +
            indent + INDENT + INDENT + "_dimensions.Wrap(buffer, parentMessage.Limit, actingVersion);\n" +
            indent + INDENT + INDENT + "_parentMessage.Limit = parentMessage.Limit + SbeHeaderSize;\n" +
            indent + INDENT + INDENT + "_blockLength = _dimensions.BlockLength;\n" +
            indent + INDENT + INDENT + "_count = (int) _dimensions.NumInGroup;\n" + // cast safety checked above
            indent + INDENT + INDENT + "_actingVersion = actingVersion;\n" +
            indent + INDENT + INDENT + "_index = 0;\n" +
            indent + INDENT + "}\n",
            parentMessageClassName));

        final int blockLength = tokens.get(index).encodedLength();
        final String typeForBlockLength = cSharpTypeName(tokens.get(index + 2).encoding().primitiveType());
        final String typeForNumInGroup = cSharpTypeName(numInGroupToken.encoding().primitiveType());

        final String throwCondition = numInGroupToken.encoding().applicableMinValue().longValue() == 0 ?
            "if ((uint) count > %3$d)\n" :
            "if (count < %2$d || count > %3$d)\n";

        sb.append(String.format("\n" +
            indent + INDENT + "public void WrapForEncode(%1$s parentMessage, DirectBuffer buffer, int count)\n" +
            indent + INDENT + "{\n" +
            indent + INDENT + INDENT + throwCondition +
            indent + INDENT + INDENT + "{\n" +
            indent + INDENT + INDENT + INDENT + "ThrowHelper.ThrowCountOutOfRangeException(count);\n" +
            indent + INDENT + INDENT + "}\n\n" +
            indent + INDENT + INDENT + "_parentMessage = parentMessage;\n" +
            indent + INDENT + INDENT + "_buffer = buffer;\n" +
            indent + INDENT + INDENT + "_dimensions.Wrap(buffer, parentMessage.Limit, SchemaVersion);\n" +
            indent + INDENT + INDENT + "parentMessage.Limit = parentMessage.Limit + SbeHeaderSize;\n" +
            indent + INDENT + INDENT + "_dimensions.BlockLength = SbeBlockLength;\n" +
            indent + INDENT + INDENT + "_dimensions.NumInGroup = (%5$s) count;\n" +
            indent + INDENT + INDENT + "_index = 0;\n" +
            indent + INDENT + INDENT + "_count = count;\n" +
            indent + INDENT + INDENT + "_blockLength = SbeBlockLength;\n" +
            indent + INDENT + INDENT + "_actingVersion = SchemaVersion;\n" +
            indent + INDENT + "}\n",
            parentMessageClassName,
            numInGroupToken.encoding().applicableMinValue().longValue(),
            numInGroupToken.encoding().applicableMaxValue().longValue(),
            typeForBlockLength,
            typeForNumInGroup));

        sb.append(String.format("\n" +
            indent + INDENT + "public const int SbeBlockLength = %d;\n" +
            indent + INDENT + "public const int SbeHeaderSize = %d;\n",
            blockLength,
            dimensionHeaderLength));

        if (null != fieldPrecedenceModel)
        {
            sb.append("\n")
                .append(indent).append("    private CodecState codecState()\n")
                .append(indent).append("    {\n")
                .append(indent).append("        return _parentMessage.codecState();\n")
                .append(indent).append("    }\n");

            sb.append("\n")
                .append(indent).append("    private void codecState(CodecState newState)\n")
                .append(indent).append("    {\n")
                .append(indent).append("        _parentMessage.codecState(newState);\n")
                .append(indent).append("    }\n");
        }

        final Token groupToken = tokens.get(index);
        generateGroupEnumerator(sb, fieldPrecedenceModel, groupToken, groupName, typeForNumInGroup, indent);
    }

    private void generateGroupEnumerator(
        final StringBuilder sb,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final Token groupToken,
        final String groupName,
        final String typeForNumInGroup,
        final String indent)
    {
        generateAccessOrderListenerMethodForNextGroupElement(sb, fieldPrecedenceModel, indent + INDENT, groupToken);
        generateAccessOrderListenerMethodForResetGroupCount(sb, fieldPrecedenceModel, indent + INDENT, groupToken);

        sb.append(
            indent + INDENT + "public int ActingBlockLength { get { return _blockLength; } }\n\n" +
            indent + INDENT + "public int Count { get { return _count; } }\n\n" +
            indent + INDENT + "public bool HasNext { get { return _index < _count; } }\n\n");

        sb.append(String.format("\n" +
            indent + INDENT + "public int ResetCountToIndex()\n" +
            indent + INDENT + "{\n" +
            "%s" +
            indent + INDENT + INDENT + "_count = _index;\n" +
            indent + INDENT + INDENT + "_dimensions.NumInGroup = (%s) _count;\n\n" +
            indent + INDENT + INDENT + "return _count;\n" +
            indent + INDENT + "}\n",
            generateAccessOrderListenerCall(fieldPrecedenceModel, indent + TWO_INDENT, "OnResetCountToIndex"),
            typeForNumInGroup));

        sb.append(String.format("\n" +
            indent + INDENT + "public %sGroup Next()\n" +
            indent + INDENT + "{\n" +
            indent + INDENT + INDENT + "if (_index >= _count)\n" +
            indent + INDENT + INDENT + "{\n" +
            indent + INDENT + INDENT + INDENT + "ThrowHelper.ThrowInvalidOperationException();\n" +
            indent + INDENT + INDENT + "}\n\n" +
            generateAccessOrderListenerCall(fieldPrecedenceModel, indent + TWO_INDENT, "OnNextElementAccessed") +
            indent + INDENT + INDENT + "_offset = _parentMessage.Limit;\n" +
            indent + INDENT + INDENT + "_parentMessage.Limit = _offset + _blockLength;\n" +
            indent + INDENT + INDENT + "++_index;\n\n" +
            indent + INDENT + INDENT + "return this;\n" +
            indent + INDENT + "}\n",
            formatClassName(groupName)));

        sb.append("\n" +
            indent + INDENT + "public System.Collections.IEnumerator GetEnumerator()\n" +
            indent + INDENT + "{\n" +
            indent + INDENT + INDENT + "while (this.HasNext)\n" +
            indent + INDENT + INDENT + "{\n" +
            indent + INDENT + INDENT + INDENT + "yield return this.Next();\n" +
            indent + INDENT + INDENT + "}\n" +
            indent + INDENT + "}\n");
    }

    private boolean isRepresentableByInt32(final Encoding encoding)
    {
        // These min and max values are the same in .NET
        return encoding.applicableMinValue().longValue() >= Integer.MIN_VALUE &&
            encoding.applicableMaxValue().longValue() <= Integer.MAX_VALUE;
    }

    private CharSequence generateGroupProperty(
        final String groupName,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final Token token,
        final String indent)
    {
        final StringBuilder sb = new StringBuilder();

        final String className = CSharpUtil.formatClassName(groupName);

        sb.append(String.format("\n" +
            indent + "private readonly %sGroup _%s = new %sGroup();\n",
            className,
            toLowerFirstChar(groupName),
            className));

        sb.append(String.format("\n" +
            indent + "public const long %sId = %d;\n",
            toUpperFirstChar(groupName),
            token.id()));

        generateAccessOrderListenerMethodForGroupWrap(sb, fieldPrecedenceModel, indent, token);

        generateSinceActingDeprecated(sb, indent, toUpperFirstChar(groupName), token);

        final String groupField = "_" + toLowerFirstChar(groupName);

        final CharSequence accessOrderListenerCallOnDecode = generateAccessOrderListenerCall(
            fieldPrecedenceModel,
            indent + TWO_INDENT,
            token,
            groupField + ".Count",
            "\"decode\"");

        sb.append(String.format("\n" +
            "%1$s" +
            indent + "public %2$sGroup %3$s\n" +
            indent + "{\n" +
            indent + INDENT + "get\n" +
            indent + INDENT + "{\n" +
            "%5$s" +

            indent + INDENT + INDENT + "_%4$s.WrapForDecode(_parentMessage, _buffer, _actingVersion);\n" +
            "%6$s" +

            indent + INDENT + INDENT + "return _%4$s;\n" +
            indent + INDENT + "}\n" +
            indent + "}\n",
            generateDocumentation(indent, token),
            className,
            toUpperFirstChar(groupName),
            toLowerFirstChar(groupName),
            generateGroupNotPresentCondition(token.version(), indent + INDENT + INDENT, groupField),
            accessOrderListenerCallOnDecode));

        final CharSequence accessOrderListenerCallOnEncode = generateAccessOrderListenerCall(
            fieldPrecedenceModel,
            indent + INDENT,
            token,
            "count",
            "\"encode\"");
        sb.append(String.format("\n" +
            indent + "public %1$sGroup %2$sCount(int count)\n" +
            indent + "{\n" +
            "%4$s" +
            indent + INDENT + "_%3$s.WrapForEncode(_parentMessage, _buffer, count);\n" +
            indent + INDENT + "return _%3$s;\n" +
            indent + "}\n",
            className,
            toUpperFirstChar(groupName),
            toLowerFirstChar(groupName),
            accessOrderListenerCallOnEncode));

        return sb;
    }

    private CharSequence generateVarData(
        final FieldPrecedenceModel fieldPrecedenceModel,
        final List<Token> tokens,
        final String indent)
    {
        final StringBuilder sb = new StringBuilder();

        for (int i = 0, size = tokens.size(); i < size; i++)
        {
            final Token token = tokens.get(i);
            if (token.signal() == Signal.BEGIN_VAR_DATA)
            {
                generateFieldIdMethod(sb, token, indent);
                generateSinceActingDeprecated(sb, indent, CSharpUtil.formatPropertyName(token.name()), token);
                generateOffsetMethod(sb, token, indent);

                final Token varDataToken = Generators.findFirst("varData", tokens, i);
                final String characterEncoding = varDataToken.encoding().characterEncoding();
                generateCharacterEncodingMethod(sb, token.name(), characterEncoding, indent);
                generateFieldMetaAttributeMethod(sb, token, indent);

                final String propertyName = toUpperFirstChar(token.name());
                final Token lengthToken = Generators.findFirst("length", tokens, i);
                final int sizeOfLengthField = lengthToken.encodedLength();
                final Encoding lengthEncoding = lengthToken.encoding();
                final String lengthCSharpType = cSharpTypeName(lengthEncoding.primitiveType());
                final String lengthTypePrefix = toUpperFirstChar(lengthEncoding.primitiveType().primitiveName());
                final ByteOrder byteOrder = lengthEncoding.byteOrder();
                final String byteOrderStr = generateByteOrder(byteOrder, lengthEncoding.primitiveType().size());

                generateAccessOrderListenerMethodForVarDataLength(sb, fieldPrecedenceModel, indent, token);
                generateAccessOrderListenerMethod(sb, fieldPrecedenceModel, indent, token);

                sb.append(String.format("\n" +
                    indent + "public const int %sHeaderSize = %d;\n",
                    propertyName,
                    sizeOfLengthField));

                final CharSequence lengthAccessOrderListenerCall = generateAccessOrderListenerCall(
                    fieldPrecedenceModel, indent + INDENT, accessOrderListenerMethodName(token, "Length"));

                sb.append(String.format(indent + "\n" +
                    indent + "public int %1$sLength()\n" +
                    indent + "{\n" +
                    "%5$s" +
                    "%6$s" +
                    indent + INDENT + "_buffer.CheckLimit(_parentMessage.Limit + %2$d);\n" +
                    indent + INDENT + "return (int)_buffer.%3$sGet%4$s(_parentMessage.Limit);\n" +
                    indent + "}\n",
                    propertyName,
                    sizeOfLengthField,
                    lengthTypePrefix,
                    byteOrderStr,
                    generateArrayFieldNotPresentCondition(token.version(), indent, "0"),
                    lengthAccessOrderListenerCall));

                final CharSequence accessOrderListenerCall =
                    generateAccessOrderListenerCall(fieldPrecedenceModel, indent + INDENT, token);

                sb.append(String.format("\n" +
                    indent + "public int Get%1$s(byte[] dst, int dstOffset, int length) =>\n" +
                    indent + INDENT + "Get%1$s(new Span<byte>(dst, dstOffset, length));\n",
                    propertyName));

                sb.append(String.format("\n" +
                    indent + "public int Get%1$s(Span<byte> dst)\n" +
                    indent + "{\n" +
                    "%2$s" +
                    "%6$s" +
                    indent + INDENT + "const int sizeOfLengthField = %3$d;\n" +
                    indent + INDENT + "int limit = _parentMessage.Limit;\n" +
                    indent + INDENT + "_buffer.CheckLimit(limit + sizeOfLengthField);\n" +
                    indent + INDENT + "int dataLength = (int)_buffer.%4$sGet%5$s(limit);\n" +
                    indent + INDENT + "int bytesCopied = Math.Min(dst.Length, dataLength);\n" +
                    indent + INDENT + "_parentMessage.Limit = limit + sizeOfLengthField + dataLength;\n" +
                    indent + INDENT + "_buffer.GetBytes(limit + sizeOfLengthField, dst.Slice(0, bytesCopied));\n\n" +
                    indent + INDENT + "return bytesCopied;\n" +
                    indent + "}\n",
                    propertyName,
                    generateArrayFieldNotPresentCondition(token.version(), indent, "0"),
                    sizeOfLengthField,
                    lengthTypePrefix,
                    byteOrderStr,
                    accessOrderListenerCall));

                sb.append(String.format(indent + "\n" +
                    indent + "// Allocates and returns a new byte array\n" +
                    indent + "public byte[] Get%1$sBytes()\n" +
                    indent + "{\n" +
                    "%5$s" +
                    "%6$s" +
                    indent + INDENT + "const int sizeOfLengthField = %2$d;\n" +
                    indent + INDENT + "int limit = _parentMessage.Limit;\n" +
                    indent + INDENT + "_buffer.CheckLimit(limit + sizeOfLengthField);\n" +
                    indent + INDENT + "int dataLength = (int)_buffer.%3$sGet%4$s(limit);\n" +
                    indent + INDENT + "byte[] data = new byte[dataLength];\n" +
                    indent + INDENT + "_parentMessage.Limit = limit + sizeOfLengthField + dataLength;\n" +
                    indent + INDENT + "_buffer.GetBytes(limit + sizeOfLengthField, data);\n\n" +
                    indent + INDENT + "return data;\n" +
                    indent + "}\n",
                    propertyName,
                    sizeOfLengthField,
                    lengthTypePrefix,
                    byteOrderStr,
                    generateArrayFieldNotPresentCondition(token.version(), indent, "new byte[0]"),
                    accessOrderListenerCall));

                sb.append(String.format("\n" +
                    indent + "public int Set%1$s(byte[] src, int srcOffset, int length) =>\n" +
                    indent + INDENT + "Set%1$s(new ReadOnlySpan<byte>(src, srcOffset, length));\n",
                    propertyName));

                sb.append(String.format("\n" +
                    indent + "public int Set%1$s(ReadOnlySpan<byte> src)\n" +
                    indent + "{\n" +
                    "%6$s" +
                    indent + INDENT + "const int sizeOfLengthField = %2$d;\n" +
                    indent + INDENT + "int limit = _parentMessage.Limit;\n" +
                    indent + INDENT + "_parentMessage.Limit = limit + sizeOfLengthField + src.Length;\n" +
                    indent + INDENT + "_buffer.%3$sPut%5$s(limit, (%4$s)src.Length);\n" +
                    indent + INDENT + "_buffer.SetBytes(limit + sizeOfLengthField, src);\n\n" +
                    indent + INDENT + "return src.Length;\n" +
                    indent + "}\n",
                    propertyName,
                    sizeOfLengthField,
                    lengthTypePrefix,
                    lengthCSharpType,
                    byteOrderStr,
                    accessOrderListenerCall));

                if (characterEncoding != null)  // only generate these string based methods if there is an encoding
                {
                    sb.append(lineSeparator())
                        .append(String.format(
                        indent + "public string Get%1$s()\n" +
                        indent + "{\n" +
                        "%6$s" +
                        "%7$s" +
                        indent + INDENT + "const int sizeOfLengthField = %2$d;\n" +
                        indent + INDENT + "int limit = _parentMessage.Limit;\n" +
                        indent + INDENT + "_buffer.CheckLimit(limit + sizeOfLengthField);\n" +
                        indent + INDENT + "int dataLength = (int)_buffer.%3$sGet%4$s(limit);\n" +
                        indent + INDENT + "_parentMessage.Limit = limit + sizeOfLengthField + dataLength;\n" +
                        indent + INDENT + "return _buffer.GetStringFromBytes(%1$sResolvedCharacterEncoding," +
                        " limit + sizeOfLengthField, dataLength);\n" +
                        indent + "}\n\n" +
                        indent + "public void Set%1$s(string value)\n" +
                        indent + "{\n" +
                        "%7$s" +
                        indent + INDENT + "var encoding = %1$sResolvedCharacterEncoding;\n" +
                        indent + INDENT + "const int sizeOfLengthField = %2$d;\n" +
                        indent + INDENT + "int limit = _parentMessage.Limit;\n" +
                        indent + INDENT + "int byteCount = _buffer.SetBytesFromString(encoding, value, " +
                        "limit + sizeOfLengthField);\n" +
                        indent + INDENT + "_parentMessage.Limit = limit + sizeOfLengthField + byteCount;\n" +
                        indent + INDENT + "_buffer.%3$sPut%4$s(limit, (%5$s)byteCount);\n" +
                        indent + "}\n",
                        propertyName,
                        sizeOfLengthField,
                        lengthTypePrefix,
                        byteOrderStr,
                        lengthCSharpType,
                        generateArrayFieldNotPresentCondition(token.version(), indent, "\"\""),
                        accessOrderListenerCall));
                }
            }
        }

        return sb;
    }

    private void generateBitSet(final List<Token> tokens) throws IOException
    {
        final Token enumToken = tokens.get(0);
        final String enumName = CSharpUtil.formatClassName(enumToken.applicableTypeName());

        try (Writer out = outputManager.createOutput(enumName))
        {
            out.append(generateFileHeader(ir.applicableNamespace()));
            out.append(generateDocumentation(INDENT, enumToken));
            final String enumPrimitiveType = cSharpTypeName(enumToken.encoding().primitiveType());
            out.append(generateEnumDeclaration(enumName, enumPrimitiveType, true));

            out.append(generateChoices(tokens.subList(1, tokens.size() - 1)));

            out.append(INDENT + "}\n");
            out.append(generateChoiceDisplay(enumName));
            out.append("}\n");
        }
    }

    private void generateEnum(final List<Token> tokens) throws IOException
    {
        final Token enumToken = tokens.get(0);
        final String enumName = CSharpUtil.formatClassName(enumToken.applicableTypeName());

        try (Writer out = outputManager.createOutput(enumName))
        {
            out.append(generateFileHeader(ir.applicableNamespace()));
            out.append(generateDocumentation(INDENT, enumToken));
            final String enumPrimitiveType = cSharpTypeName(enumToken.encoding().primitiveType());
            out.append(generateEnumDeclaration(enumName, enumPrimitiveType, false));

            out.append(generateEnumValues(tokens.subList(1, tokens.size() - 1), enumToken));

            out.append(INDENT + "}\n");
            out.append("}\n");
        }
    }

    private void generateComposite(final List<Token> tokens) throws IOException
    {
        final String compositeName = CSharpUtil.formatClassName(tokens.get(0).applicableTypeName());

        try (Writer out = outputManager.createOutput(compositeName))
        {
            out.append(generateFileHeader(ir.applicableNamespace()));
            out.append(generateDocumentation(INDENT, tokens.get(0)));
            out.append(generateClassDeclaration(compositeName));
            out.append(generateFixedFlyweightCode(tokens.get(0).encodedLength()));
            out.append(generateCompositePropertyElements(tokens.subList(1, tokens.size() - 1), BASE_INDENT));

            out.append(generateCompositeDisplay(tokens));
            out.append(INDENT + "}\n");
            out.append("}\n");
        }
    }

    private CharSequence generateCompositePropertyElements(final List<Token> tokens, final String indent)
    {
        final StringBuilder sb = new StringBuilder();

        for (int i = 0; i < tokens.size();)
        {
            final Token token = tokens.get(i);
            final String propertyName = formatPropertyName(token.name());

            switch (token.signal())
            {
                case ENCODING:
                    sb.append(generatePrimitiveProperty(propertyName, token, token, null, indent));
                    break;

                case BEGIN_ENUM:
                    sb.append(generateEnumProperty(propertyName, token, token, null, indent));
                    break;

                case BEGIN_SET:
                    sb.append(generateBitSetProperty(propertyName, token, token, null, indent));
                    break;

                case BEGIN_COMPOSITE:
                    sb.append(generateCompositeProperty(propertyName, token, token, null, indent));
                    break;

                default:
                    break;
            }

            i += tokens.get(i).componentTokenCount();
        }

        return sb;
    }

    private CharSequence generateChoices(final List<Token> tokens)
    {
        final StringBuilder sb = new StringBuilder();

        for (final Token token : tokens)
        {
            if (token.signal() == Signal.CHOICE)
            {
                final String choiceName = toUpperFirstChar(token.applicableTypeName());
                final String choiceBitPosition = token.encoding().constValue().toString();
                final int choiceValue = (int)Math.pow(2, Integer.parseInt(choiceBitPosition));
                sb.append(String.format(INDENT + INDENT + "%s = %s,\n", choiceName, choiceValue));
            }
        }

        return sb;
    }

    private CharSequence generateEnumValues(final List<Token> tokens, final Token encodingToken)
    {
        final StringBuilder sb = new StringBuilder();
        final Encoding encoding = encodingToken.encoding();

        for (final Token token : tokens)
        {
            sb.append(generateDocumentation(INDENT + INDENT, token))
              .append(INDENT).append(INDENT).append(token.name()).append(" = ")
              .append(token.encoding().constValue()).append(",\n");
        }

        final PrimitiveValue nullVal = encoding.applicableNullValue();

        sb.append(INDENT).append(INDENT).append("NULL_VALUE = ").append(nullVal).append("\n");

        return sb;
    }

    private CharSequence generateFileHeader(final String packageName)
    {
        String[] tokens = packageName.split("\\.");
        final StringBuilder sb = new StringBuilder();
        for (final String t : tokens)
        {
            sb.append(toUpperFirstChar(t)).append(".");
        }
        if (sb.length() > 0)
        {
            sb.setLength(sb.length() - 1);
        }

        tokens = sb.toString().split("-");
        sb.setLength(0);

        for (final String t : tokens)
        {
            sb.append(toUpperFirstChar(t));
        }

        return String.format(
            "// <auto-generated>\n" +
            "//     Generated SBE (Simple Binary Encoding) message codec\n" +
            "// </auto-generated>\n\n" +
            "#pragma warning disable 1591 // disable warning on missing comments\n" +
            "using System;\n" +
            "using System.Text;\n" +
            "using Org.SbeTool.Sbe.Dll;\n\n" +
            "namespace %s\n" +
            "{\n",
            sb);
    }

    private CharSequence generateClassDeclaration(final String className)
    {
        return String.format(
            INDENT + "public sealed partial class %s\n" +
            INDENT + "{\n",
            className);
    }

    private static String generateDocumentation(final String indent, final Token token)
    {
        final String description = token.description();
        if (null == description || description.isEmpty())
        {
            return "";
        }

        return
            indent + "/// <summary>\n" +
            indent + "/// " + description + "\n" +
            indent + "/// </summary>\n";
    }

    private void generateMetaAttributeEnum() throws IOException
    {
        try (Writer out = outputManager.createOutput(META_ATTRIBUTE_ENUM))
        {
            out.append(generateFileHeader(ir.applicableNamespace()));

            out.append(
                INDENT + "public enum MetaAttribute\n" +
                INDENT + "{\n" +
                INDENT + INDENT + "Epoch,\n" +
                INDENT + INDENT + "TimeUnit,\n" +
                INDENT + INDENT + "SemanticType,\n" +
                INDENT + INDENT + "Presence\n" +
                INDENT + "}\n" +
                "}\n");
        }
    }

    private CharSequence generateEnumDeclaration(
        final String name,
        final String primitiveType,
        final boolean addFlagsAttribute)
    {
        String result = "";
        if (addFlagsAttribute)
        {
            result += INDENT + "[Flags]\n";
        }

        result +=
            INDENT + "public enum " + name + " : " + primitiveType + "\n" +
            INDENT + "{\n";

        return result;
    }

    private CharSequence generatePrimitiveProperty(
        final String propertyName,
        final Token fieldToken,
        final Token typeToken,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent)
    {
        final StringBuilder sb = new StringBuilder();

        sb.append(generatePrimitiveFieldMetaData(propertyName, typeToken, indent + INDENT));

        if (typeToken.isConstantEncoding())
        {
            sb.append(generateConstPropertyMethods(propertyName, typeToken, indent));
        }
        else
        {
            sb.append(generatePrimitivePropertyMethods(propertyName, fieldToken, typeToken,
                fieldPrecedenceModel, indent));
        }

        return sb;
    }

    private CharSequence generatePrimitivePropertyMethods(
        final String propertyName,
        final Token fieldToken,
        final Token typeToken,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent)
    {
        final int arrayLength = typeToken.arrayLength();

        if (arrayLength == 1)
        {
            return generateSingleValueProperty(propertyName, fieldToken, typeToken,
                fieldPrecedenceModel, indent + INDENT);
        }
        else if (arrayLength > 1)
        {
            return generateArrayProperty(propertyName, fieldToken, typeToken,
                fieldPrecedenceModel, indent + INDENT);
        }

        return "";
    }

    private CharSequence generatePrimitiveFieldMetaData(
        final String propertyName,
        final Token token,
        final String indent)
    {
        final PrimitiveType primitiveType = token.encoding().primitiveType();
        final String typeName = cSharpTypeName(primitiveType);

        return String.format(
            "\n" +
            indent + "public const %1$s %2$sNullValue = %3$s;\n" +
            indent + "public const %1$s %2$sMinValue = %4$s;\n" +
            indent + "public const %1$s %2$sMaxValue = %5$s;\n",
            typeName,
            toUpperFirstChar(propertyName),
            generateLiteral(primitiveType, token.encoding().applicableNullValue().toString()),
            generateLiteral(primitiveType, token.encoding().applicableMinValue().toString()),
            generateLiteral(primitiveType, token.encoding().applicableMaxValue().toString()));
    }

    private CharSequence generateSingleValueProperty(
        final String propertyName,
        final Token fieldToken,
        final Token typeToken,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent)
    {
        final String typeName = cSharpTypeName(typeToken.encoding().primitiveType());
        final String typePrefix = toUpperFirstChar(typeToken.encoding().primitiveType().primitiveName());
        final int offset = typeToken.offset();
        final ByteOrder byteOrder = typeToken.encoding().byteOrder();
        final String byteOrderStr = generateByteOrder(byteOrder, typeToken.encoding().primitiveType().size());

        final CharSequence accessOrderListenerCall =
            generateAccessOrderListenerCall(fieldPrecedenceModel, indent + TWO_INDENT, fieldToken);

        return String.format("\n" +
            "%1$s" +
            indent + "public %2$s %3$s\n" +
            indent + "{\n" +
            indent + INDENT + "get\n" +
            indent + INDENT + "{\n" +
            "%4$s" +
            "%8$s" +
            indent + INDENT + INDENT + "return _buffer.%5$sGet%7$s(_offset + %6$d);\n" +
            indent + INDENT + "}\n" +
            indent + INDENT + "set\n" +
            indent + INDENT + "{\n" +
            "%8$s" +
            indent + INDENT + INDENT + "_buffer.%5$sPut%7$s(_offset + %6$d, value);\n" +
            indent + INDENT + "}\n" +
            indent + "}\n\n",
            generateDocumentation(indent, fieldToken),
            typeName,
            toUpperFirstChar(propertyName),
            generateFieldNotPresentCondition(fieldToken.version(), typeToken.encoding(), indent),
            typePrefix,
            offset,
            byteOrderStr,
            accessOrderListenerCall);
    }

    private CharSequence generateFieldNotPresentCondition(
        final int sinceVersion,
        final Encoding encoding,
        final String indent)
    {
        if (0 == sinceVersion)
        {
            return "";
        }

        final String literal;
        if (sinceVersion > 0)
        {
            literal = generateLiteral(encoding.primitiveType(), encoding.applicableNullValue().toString());
        }
        else
        {
            literal = "(byte)0";
        }

        return String.format(
            indent + INDENT + INDENT + "if (_actingVersion < %1$d) return %2$s;\n\n",
            sinceVersion,
            literal);
    }

    private CharSequence generateGroupNotPresentCondition(
        final int sinceVersion,
        final String indent,
        final String groupInstanceField)
    {
        if (0 == sinceVersion)
        {
            return "";
        }

        return indent + "if (_actingVersion < " + sinceVersion + ")" +
            indent + "{\n" +
            indent + INDENT + groupInstanceField + ".NotPresent();\n" +
            indent + INDENT + "return " + groupInstanceField + ";\n" +
            indent + "}\n\n";
    }

    private CharSequence generateArrayFieldNotPresentCondition(
        final int sinceVersion,
        final String indent,
        final String defaultValue)
    {
        if (0 == sinceVersion)
        {
            return "";
        }

        return indent + INDENT + "if (_actingVersion < " + sinceVersion + ") return " + defaultValue + ";\n\n";
    }

    private CharSequence generateBitSetNotPresentCondition(
        final int sinceVersion,
        final String indent,
        final String bitSetName)
    {
        if (0 == sinceVersion)
        {
            return "";
        }

        return String.format(
            indent + INDENT + INDENT + INDENT + "if (_actingVersion < %1$d) return (%2$s)0;\n\n",
            sinceVersion,
            bitSetName);
    }

    private CharSequence generateTypeFieldNotPresentCondition(
        final int sinceVersion,
        final String indent)
    {
        if (0 == sinceVersion)
        {
            return "";
        }

        return String.format(
            indent + INDENT + INDENT + "if (_actingVersion < %d) return null;\n\n",
            sinceVersion);
    }

    private CharSequence generateArrayProperty(
        final String propertyName,
        final Token fieldToken,
        final Token typeToken,
        final FieldPrecedenceModel fieldPrecedenceModel, final String indent)
    {
        final String typeName = cSharpTypeName(typeToken.encoding().primitiveType());
        final String typePrefix = toUpperFirstChar(typeToken.encoding().primitiveType().primitiveName());
        final int offset = typeToken.offset();
        final ByteOrder byteOrder = typeToken.encoding().byteOrder();
        final String byteOrderStr = generateByteOrder(byteOrder, typeToken.encoding().primitiveType().size());
        final int fieldLength = typeToken.arrayLength();
        final int typeSize = typeToken.encoding().primitiveType().size();
        final String propName = toUpperFirstChar(propertyName);

        final CharSequence accessOrderListenerCall =
            generateAccessOrderListenerCall(fieldPrecedenceModel, indent + INDENT, fieldToken);
        final CharSequence accessOrderListenerCallDoubleIndent =
            generateAccessOrderListenerCall(fieldPrecedenceModel, indent + TWO_INDENT, fieldToken);

        final StringBuilder sb = new StringBuilder();

        sb.append(String.format("\n" +
            indent + "public const int %sLength = %d;\n",
            propName, fieldLength));

        sb.append(String.format("\n" +
            "%1$s" +
            indent + "public %2$s Get%3$s(int index)\n" +
            indent + "{\n" +
            indent + INDENT + "if ((uint) index >= %4$d)\n" +
            indent + INDENT + "{\n" +
            indent + INDENT + INDENT + "ThrowHelper.ThrowIndexOutOfRangeException(index);\n" +
            indent + INDENT + "}\n\n" +
            "%5$s" +
            "%10$s" +
            indent + INDENT + "return _buffer.%6$sGet%9$s(_offset + %7$d + (index * %8$d));\n" +
            indent + "}\n",
            generateDocumentation(indent, fieldToken),
            typeName, propName, fieldLength,
            generateFieldNotPresentCondition(fieldToken.version(), typeToken.encoding(), indent),
            typePrefix, offset, typeSize, byteOrderStr,
            accessOrderListenerCall));

        sb.append(String.format("\n" +
            "%1$s" +
            indent + "public void Set%2$s(int index, %3$s value)\n" +
            indent + "{\n" +
            indent + INDENT + "if ((uint) index >= %4$d)\n" +
            indent + INDENT + "{\n" +
            indent + INDENT + INDENT + "ThrowHelper.ThrowIndexOutOfRangeException(index);\n" +
            indent + INDENT + "}\n\n" +
            "%9$s" +
            indent + INDENT + "_buffer.%5$sPut%8$s(_offset + %6$d + (index * %7$d), value);\n" +
            indent + "}\n",
            generateDocumentation(indent, fieldToken),
            propName, typeName, fieldLength, typePrefix, offset, typeSize, byteOrderStr,
            accessOrderListenerCall));

        sb.append(String.format("\n" +
            "%1$s" +
            indent + "public ReadOnlySpan<%2$s> %3$s\n" +
            indent + "{\n" +
            indent + INDENT + "get\n" +
            indent + INDENT + "{\n" +
            "%5$s" +
            "%6$s" +
            indent + INDENT + INDENT + "return _buffer.AsReadOnlySpan<%2$s>(_offset + %4$s, %3$sLength);\n" +
            indent + INDENT + "}\n" +
            indent + INDENT + "set\n" +
            indent + INDENT + "{\n" +
            "%6$s" +
            indent + INDENT + INDENT + "value.CopyTo(_buffer.AsSpan<%2$s>(_offset + %4$s, %3$sLength));\n" +
            indent + INDENT + "}\n" +
            indent + "}\n",
            generateDocumentation(indent, fieldToken),
            typeName, propName, offset,
            generateArrayFieldNotPresentCondition(fieldToken.version(),
            indent + INDENT + INDENT, "new " + typeName + "[0]"),
            accessOrderListenerCallDoubleIndent));

        sb.append(String.format("\n" +
            "%1$s" +
            indent + "public Span<%2$s> %3$sAsSpan()\n" +
            indent + "{\n" +
            "%5$s" +
            "%6$s" +
            indent + INDENT + "return _buffer.AsSpan<%2$s>(_offset + %4$s, %3$sLength);\n" +
            indent + "}\n",
            generateDocumentation(indent, fieldToken),
            typeName, propName, offset,
            generateArrayFieldNotPresentCondition(fieldToken.version(),
            indent + INDENT + INDENT, "new " + typeName + "[0]"),
            accessOrderListenerCall));

        if (typeToken.encoding().primitiveType() == PrimitiveType.CHAR)
        {
            generateCharacterEncodingMethod(sb, propertyName, typeToken.encoding().characterEncoding(), indent);

            sb.append(String.format("\n" +
                indent + "public int Get%1$s(byte[] dst, int dstOffset)\n" +
                indent + "{\n" +
                indent + INDENT + "const int length = %2$d;\n" +
                "%3$s" +
                "%4$s" +
                indent + INDENT + "return Get%1$s(new Span<byte>(dst, dstOffset, length));\n" +
                indent + "}\n",
                propName,
                fieldLength,
                generateArrayFieldNotPresentCondition(fieldToken.version(), indent, "0"),
                accessOrderListenerCall));

            sb.append(String.format("\n" +
                indent + "public int Get%1$s(Span<byte> dst)\n" +
                indent + "{\n" +
                indent + INDENT + "const int length = %2$d;\n" +
                indent + INDENT + "if (dst.Length < length)\n" +
                indent + INDENT + "{\n" +
                indent + INDENT + INDENT + "ThrowHelper.ThrowWhenSpanLengthTooSmall(dst.Length);\n" +
                indent + INDENT + "}\n\n" +
                "%3$s" +
                "%5$s" +
                indent + INDENT + "_buffer.GetBytes(_offset + %4$d, dst);\n" +
                indent + INDENT + "return length;\n" +
                indent + "}\n",
                propName,
                fieldLength,
                generateArrayFieldNotPresentCondition(fieldToken.version(), indent, "0"),
                offset,
                accessOrderListenerCall));

            sb.append(String.format("\n" +
                indent + "public void Set%1$s(byte[] src, int srcOffset)\n" +
                indent + "{\n" +
                indent + INDENT + "Set%1$s(new ReadOnlySpan<byte>(src, srcOffset, src.Length - srcOffset));\n" +
                indent + "}\n",
                propName, fieldLength, offset));

            sb.append(String.format("\n" +
                indent + "public void Set%1$s(ReadOnlySpan<byte> src)\n" +
                indent + "{\n" +
                indent + INDENT + "const int length = %2$d;\n" +
                indent + INDENT + "if (src.Length > length)\n" +
                indent + INDENT + "{\n" +
                indent + INDENT + INDENT + "ThrowHelper.ThrowWhenSpanLengthTooLarge(src.Length);\n" +
                indent + INDENT + "}\n\n" +
                "%4$s" +
                indent + INDENT + "_buffer.SetBytes(_offset + %3$d, src);\n" +
                indent + "}\n",
                propName, fieldLength, offset,
                accessOrderListenerCall));

            sb.append(String.format("\n" +
                indent + "public void Set%1$s(string value)\n" +
                indent + "{\n" +
                "%3$s" +
                indent + INDENT + "_buffer.SetNullTerminatedBytesFromString(%1$sResolvedCharacterEncoding, " +
                "value, _offset + %2$s, %1$sLength, %1$sNullValue);\n" +
                indent + "}\n" +
                indent + "public string Get%1$s()\n" +
                indent + "{\n" +
                "%3$s" +
                indent + INDENT + "return _buffer.GetStringFromNullTerminatedBytes(%1$sResolvedCharacterEncoding, " +
                "_offset + %2$s, %1$sLength, %1$sNullValue);\n" +
                indent + "}\n",
                propName,
                offset,
                accessOrderListenerCall));
        }

        return sb;
    }

    private void generateCharacterEncodingMethod(
        final StringBuilder sb,
        final String propertyName,
        final String encoding,
        final String indent)
    {
        if (encoding != null) // Raw data fields might not have encodings
        {
            sb.append(String.format("\n" +
                indent + "public const string %1$sCharacterEncoding = \"%2$s\";\n" +
                indent + "public static Encoding %1$sResolvedCharacterEncoding = " +
                "Encoding.GetEncoding(%1$sCharacterEncoding);\n\n",
                formatPropertyName(propertyName), encoding));
        }
    }

    private CharSequence generateConstPropertyMethods(
        final String propertyName,
        final Token token,
        final String indent)
    {
        if (token.encoding().primitiveType() != PrimitiveType.CHAR)
        {
            // ODE: we generate a property here because the constant could
            // become a field in a newer version of the protocol
            return String.format("\n" +
                "%1s" +
                indent + INDENT + "public %2$s %3$s { get { return %4$s; } }\n",
                generateDocumentation(indent + INDENT, token),
                cSharpTypeName(token.encoding().primitiveType()),
                toUpperFirstChar(propertyName),
                generateLiteral(token.encoding().primitiveType(), token.encoding().constValue().toString()));
        }

        final StringBuilder sb = new StringBuilder();

        final String javaTypeName = cSharpTypeName(token.encoding().primitiveType());
        final byte[] constantValue = token.encoding().constValue().byteArrayValue(token.encoding().primitiveType());
        final CharSequence values = generateByteLiteralList(
            token.encoding().constValue().byteArrayValue(token.encoding().primitiveType()));

        sb.append(String.format(
            "\n" +
            indent + INDENT + "private static readonly byte[] _%1$sValue = { %2$s };\n",
            propertyName,
            values));

        sb.append(String.format(
            "\n" +
            indent + INDENT + "public const int %1$sLength = %2$d;\n",
            toUpperFirstChar(propertyName),
            constantValue.length));

        sb.append(String.format(
            indent + INDENT + "public %1$s %2$s(int index)\n" +
            indent + INDENT + "{\n" +
            indent + INDENT + INDENT + "return _%3$sValue[index];\n" +
            indent + INDENT + "}\n\n",
            javaTypeName,
            toUpperFirstChar(propertyName),
            propertyName));

        sb.append(String.format(
            indent + INDENT + "public int Get%1$s(byte[] dst, int offset, int length)\n" +
            indent + INDENT + "{\n" +
            indent + INDENT + INDENT + "int bytesCopied = Math.Min(length, %2$d);\n" +
            indent + INDENT + INDENT + "Array.Copy(_%3$sValue, 0, dst, offset, bytesCopied);\n" +
            indent + INDENT + INDENT + "return bytesCopied;\n" +
            indent + INDENT + "}\n",
            toUpperFirstChar(propertyName),
            constantValue.length,
            propertyName));

        return sb;
    }

    private CharSequence generateByteLiteralList(final byte[] bytes)
    {
        final StringBuilder values = new StringBuilder();
        for (final byte b : bytes)
        {
            values.append(b).append(", ");
        }

        if (values.length() > 0)
        {
            values.setLength(values.length() - 2);
        }

        return values;
    }

    private CharSequence generateFixedFlyweightCode(final int size)
    {
        return String.format(
            INDENT + INDENT + "public const %1$s SbeSchemaId = %2$s;\n" +
            INDENT + INDENT + "public const %3$s SbeSchemaVersion = %4$s;\n" +
            INDENT + INDENT + "public const int Size = %5$d;\n\n" +

            INDENT + INDENT + "private DirectBuffer _buffer;\n" +
            INDENT + INDENT + "private int _offset;\n" +
            INDENT + INDENT + "private int _actingVersion;\n\n" +

            INDENT + INDENT + "public void Wrap(DirectBuffer buffer, int offset, int actingVersion)\n" +
            INDENT + INDENT + "{\n" +
            INDENT + INDENT + INDENT + "_offset = offset;\n" +
            INDENT + INDENT + INDENT + "_actingVersion = actingVersion;\n" +
            INDENT + INDENT + INDENT + "_buffer = buffer;\n" +
            INDENT + INDENT + "}\n\n",
            cSharpTypeName(ir.headerStructure().schemaIdType()),
            generateLiteral(ir.headerStructure().schemaIdType(), Integer.toString(ir.id())),
            cSharpTypeName(ir.headerStructure().schemaVersionType()),
            generateLiteral(ir.headerStructure().schemaVersionType(), Integer.toString(ir.version())),
            size);
    }

    private CharSequence generateMessageFlyweightCode(
        final String className,
        final Token token,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent)
    {
        final String blockLengthType = cSharpTypeName(ir.headerStructure().blockLengthType());
        final String templateIdType = cSharpTypeName(ir.headerStructure().templateIdType());
        final String schemaIdType = cSharpTypeName(ir.headerStructure().schemaIdType());
        final String schemaVersionType = cSharpTypeName(ir.headerStructure().schemaVersionType());
        final String semanticType = token.encoding().semanticType() == null ? "" : token.encoding().semanticType();
        final String semanticVersion = ir.semanticVersion() == null ? "" : ir.semanticVersion();

        return String.format(
            indent + INDENT + "public const %1$s BlockLength = %2$s;\n" +
            indent + INDENT + "public const %3$s TemplateId = %4$s;\n" +
            indent + INDENT + "public const %5$s SchemaId = %6$s;\n" +
            indent + INDENT + "public const %7$s SchemaVersion = %8$s;\n" +
            indent + INDENT + "public const string SemanticType = \"%9$s\";\n" +
            indent + INDENT + "public const string SemanticVersion = \"%11$s\";\n\n" +
            indent + INDENT + "private readonly %10$s _parentMessage;\n" +
            indent + INDENT + "private DirectBuffer _buffer;\n" +
            indent + INDENT + "private int _offset;\n" +
            indent + INDENT + "private int _limit;\n" +
            indent + INDENT + "private int _actingBlockLength;\n" +
            indent + INDENT + "private int _actingVersion;\n" +
            "\n" +
            indent + INDENT + "public int Offset { get { return _offset; } }\n\n" +
            indent + INDENT + "public %10$s()\n" +
            indent + INDENT + "{\n" +
            indent + INDENT + INDENT + "_parentMessage = this;\n" +
            indent + INDENT + "}\n\n" +
            indent + INDENT + "public %10$s WrapForEncode(DirectBuffer buffer, int offset)\n" +
            indent + INDENT + "{\n" +
            "%12$s" +
            indent + INDENT + INDENT + "_buffer = buffer;\n" +
            indent + INDENT + INDENT + "_offset = offset;\n" +
            indent + INDENT + INDENT + "_actingBlockLength = BlockLength;\n" +
            indent + INDENT + INDENT + "_actingVersion = SchemaVersion;\n" +
            indent + INDENT + INDENT + "Limit = offset + _actingBlockLength;\n" +
            indent + INDENT + INDENT + "return this;\n" +
            indent + INDENT + "}\n\n" +
            indent + INDENT + "public %10$s WrapForEncodeAndApplyHeader(DirectBuffer buffer, int offset, " +
                "MessageHeader headerEncoder)\n" +
            indent + INDENT + "{\n" +
            indent + INDENT + INDENT + "headerEncoder.Wrap(buffer, offset, SchemaVersion);\n" +
            indent + INDENT + INDENT + "headerEncoder.BlockLength = BlockLength;\n" +
            indent + INDENT + INDENT + "headerEncoder.TemplateId = TemplateId;\n" +
            indent + INDENT + INDENT + "headerEncoder.SchemaId = SchemaId;\n" +
            indent + INDENT + INDENT + "headerEncoder.Version = SchemaVersion;\n" +
            indent + INDENT + INDENT + "\n" +
            indent + INDENT + INDENT + "return WrapForEncode(buffer, offset + MessageHeader.Size);\n" +
            indent + INDENT + "}\n\n" +
            "%13$s" +
            indent + INDENT + "public %10$s WrapForDecode(DirectBuffer buffer, int offset, " +
                "int actingBlockLength, int actingVersion)\n" +
            indent + INDENT + "{\n" +
            "%14$s" +
            indent + INDENT + INDENT + "_buffer = buffer;\n" +
            indent + INDENT + INDENT + "_offset = offset;\n" +
            indent + INDENT + INDENT + "_actingBlockLength = actingBlockLength;\n" +
            indent + INDENT + INDENT + "_actingVersion = actingVersion;\n" +
            indent + INDENT + INDENT + "Limit = offset + _actingBlockLength;\n" +
            indent + INDENT + INDENT + "return this;\n" +
            indent + INDENT + "}\n\n" +
            indent + INDENT + "public %10$s WrapForDecodeAndApplyHeader(DirectBuffer buffer, int offset, " +
                "MessageHeader headerDecoder)\n" +
            indent + INDENT + "{\n" +
            indent + INDENT + INDENT + "headerDecoder.Wrap(buffer, offset, SchemaVersion);\n" +
            indent + INDENT + INDENT + "\n" +
            indent + INDENT + INDENT + "return WrapForDecode(buffer, offset + MessageHeader.Size, " +
            " headerDecoder.BlockLength, headerDecoder.Version);\n" +
            indent + INDENT + "}\n\n" +
            indent + INDENT + "public int Size\n" +
            indent + INDENT + "{\n" +
            indent + INDENT + INDENT + "get\n" +
            indent + INDENT + INDENT + "{\n" +
            indent + INDENT + INDENT + INDENT + "return _limit - _offset;\n" +
            indent + INDENT + INDENT + "}\n" +
            indent + INDENT + "}\n\n" +
            indent + INDENT + "public int Limit\n" +
            indent + INDENT + "{\n" +
            indent + INDENT + INDENT + "get\n" +
            indent + INDENT + INDENT + "{\n" +
            indent + INDENT + INDENT + INDENT + "return _limit;\n" +
            indent + INDENT + INDENT + "}\n" +
            indent + INDENT + INDENT + "set\n" +
            indent + INDENT + INDENT + "{\n" +
            indent + INDENT + INDENT + INDENT + "_buffer.CheckLimit(value);\n" +
            indent + INDENT + INDENT + INDENT + "_limit = value;\n" +
            indent + INDENT + INDENT + "}\n" +
            indent + INDENT + "}\n\n",
            blockLengthType,
            generateLiteral(ir.headerStructure().blockLengthType(), Integer.toString(token.encodedLength())),
            templateIdType,
            generateLiteral(ir.headerStructure().templateIdType(), Integer.toString(token.id())),
            schemaIdType,
            generateLiteral(ir.headerStructure().schemaIdType(), Integer.toString(ir.id())),
            schemaVersionType,
            generateLiteral(ir.headerStructure().schemaVersionType(), Integer.toString(ir.version())),
            semanticType,
            className,
            semanticVersion,
            generateEncoderWrapListener(fieldPrecedenceModel, indent + TWO_INDENT),
            generateDecoderWrapListener(fieldPrecedenceModel, indent + INDENT),
            generateAccessOrderListenerCall(fieldPrecedenceModel, indent + TWO_INDENT,
                "OnWrapForDecode", "actingVersion"));
    }

    private static CharSequence qualifiedStateCase(final FieldPrecedenceModel.State state)
    {
        return "CodecState." + state.name();
    }

    private static CharSequence stateCaseForSwitchCase(final FieldPrecedenceModel.State state)
    {
        return qualifiedStateCase(state);
    }

    private static CharSequence unqualifiedStateCase(final FieldPrecedenceModel.State state)
    {
        return state.name();
    }

    private static CharSequence generateFieldOrderStates(
        final String indent,
        final FieldPrecedenceModel fieldPrecedenceModel)
    {
        if (null == fieldPrecedenceModel)
        {
            return "";
        }

        final StringBuilder sb = new StringBuilder();

        sb.append(indent).append("///\n");

        sb.append(indent).append("/// <summary>\n");
        sb.append(indent).append("///   <para>\n");
        sb.append(indent).append("///     The states in which a encoder/decoder/codec can live.\n");
        sb.append(indent).append("///   </para>\n");
        sb.append(indent).append("///   <para>\n");
        sb.append(indent).append("///     The state machine diagram below, encoded in the dot language, describes\n");
        sb.append(indent).append("///     the valid state transitions according to the order in which fields may be\n");
        sb.append(indent).append("///     accessed safely. Tools such as PlantUML and Graphviz can render it.\n");
        sb.append(indent).append("///   </para>\n");
        sb.append(indent).append("///   <code>\n");
        fieldPrecedenceModel.generateGraph(sb, indent + "///     ");
        sb.append(indent).append("///   </code>\n");
        sb.append(indent).append("/// </summary>\n");
        sb.append(indent).append("private enum CodecState\n")
            .append(indent).append("{\n");
        fieldPrecedenceModel.forEachStateOrderedByStateNumber((state) ->
            sb.append(indent).append(INDENT).append(unqualifiedStateCase(state))
            .append(" = ").append(state.number())
            .append(",\n"));
        sb.append(indent).append("}\n\n");

        sb.append("\n").append(indent).append("private static readonly string[] StateNameLookup = new []\n")
            .append(indent).append("{\n");
        fieldPrecedenceModel.forEachStateOrderedByStateNumber((state) ->
            sb.append(indent).append(INDENT).append("\"").append(state.name()).append("\",\n"));
        sb.append(indent).append("};\n\n");

        sb.append(indent).append("private static readonly string[] StateTransitionsLookup = new []\n")
            .append(indent).append("{\n");
        fieldPrecedenceModel.forEachStateOrderedByStateNumber((state) ->
        {
            sb.append(indent).append(INDENT).append("\"");
            final MutableBoolean isFirst = new MutableBoolean(true);
            final Set<String> transitionDescriptions = new HashSet<>();
            fieldPrecedenceModel.forEachTransitionFrom(state, (transitionGroup) ->
            {
                if (transitionDescriptions.add(transitionGroup.exampleCode()))
                {
                    if (isFirst.get())
                    {
                        isFirst.set(false);
                    }
                    else
                    {
                        sb.append(", ");
                    }

                    sb.append("\\\"").append(transitionGroup.exampleCode()).append("\\\"");
                }
            });
            sb.append("\",\n");
        });
        sb.append(indent).append("};\n\n");

        sb.append(indent).append("private static string codecStateName(CodecState state)\n")
            .append(indent).append("{\n")
            .append(indent).append(INDENT).append("return StateNameLookup[(int) state];\n")
            .append(indent).append("}\n\n");

        sb.append(indent).append("private static string codecStateTransitions(CodecState state)\n")
            .append(indent).append("{\n")
            .append(indent).append(INDENT).append("return StateTransitionsLookup[(int) state];\n")
            .append(indent).append("}\n\n");

        sb.append(indent).append("private CodecState _codecState = ")
            .append(qualifiedStateCase(fieldPrecedenceModel.notWrappedState()))
            .append(";\n\n");

        sb.append(indent).append("private CodecState codecState()\n")
            .append(indent).append("{\n")
            .append(indent).append(INDENT).append("return _codecState;\n")
            .append(indent).append("}\n\n");

        sb.append(indent).append("private void codecState(CodecState newState)\n")
            .append(indent).append("{\n")
            .append(indent).append(INDENT).append("_codecState = newState;\n")
            .append(indent).append("}\n");

        return sb;
    }

    private CharSequence generateFullyEncodedCheck(
        final String indent,
        final FieldPrecedenceModel fieldPrecedenceModel)
    {
        if (null == fieldPrecedenceModel)
        {
            return "";
        }

        final StringBuilder sb = new StringBuilder();
        sb.append("\n");

        sb.append(indent).append("public void CheckEncodingIsComplete()\n")
            .append(indent).append("{\n")
            .append("#if ").append(precedenceChecksFlagName).append("\n")
            .append(indent).append(INDENT).append("switch (_codecState)\n")
            .append(indent).append(INDENT).append("{\n");

        fieldPrecedenceModel.forEachTerminalEncoderState((state) ->
            sb.append(indent).append(TWO_INDENT).append("case ").append(stateCaseForSwitchCase(state)).append(":\n")
            .append(indent).append(THREE_INDENT).append("return;\n"));

        sb.append(indent).append(TWO_INDENT).append("default:\n")
            .append(indent).append(THREE_INDENT)
            .append("throw new InvalidOperationException(\"Not fully encoded, current state: \" +\n")
            .append(indent).append(THREE_INDENT)
            .append(INDENT).append("codecStateName(_codecState) + \", allowed transitions: \" +\n")
            .append(indent).append(THREE_INDENT)
            .append(INDENT).append("codecStateTransitions(_codecState));\n")
            .append(indent).append(INDENT).append("}\n")
            .append("#endif\n")
            .append(indent).append("}\n\n");

        return sb;
    }

    private static String accessOrderListenerMethodName(final Token token)
    {
        return "On" + Generators.toUpperFirstChar(token.name()) + "Accessed";
    }

    private static String accessOrderListenerMethodName(final Token token, final String suffix)
    {
        return "On" + Generators.toUpperFirstChar(token.name()) + suffix + "Accessed";
    }

    private static void generateAccessOrderListenerMethod(
        final StringBuilder sb,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent,
        final Token token)
    {
        if (null == fieldPrecedenceModel)
        {
            return;
        }

        sb.append("\n")
            .append(indent).append("private void ").append(accessOrderListenerMethodName(token)).append("()\n")
            .append(indent).append("{\n");

        final FieldPrecedenceModel.CodecInteraction fieldAccess =
            fieldPrecedenceModel.interactionFactory().accessField(token);

        generateAccessOrderListener(
            sb,
            indent + INDENT,
            "access field",
            fieldPrecedenceModel,
            fieldAccess);

        sb.append(indent).append("}\n");
    }

    private CharSequence generateAccessOrderListenerCall(
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent,
        final Token token,
        final String... arguments)
    {
        return generateAccessOrderListenerCall(
            fieldPrecedenceModel,
            indent,
            accessOrderListenerMethodName(token),
            arguments);
    }

    private CharSequence generateAccessOrderListenerCall(
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent,
        final String methodName,
        final String... arguments)
    {
        if (null == fieldPrecedenceModel)
        {
            return "";
        }

        final StringBuilder sb = new StringBuilder();
        sb.append("#if ").append(precedenceChecksFlagName).append("\n")
            .append(indent).append(methodName).append("(");

        for (int i = 0; i < arguments.length; i++)
        {
            if (i > 0)
            {
                sb.append(", ");
            }
            sb.append(arguments[i]);
        }
        sb.append(");\n");

        sb.append("#endif\n");

        return sb;
    }

    private static void generateAccessOrderListenerMethodForGroupWrap(
        final StringBuilder sb,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent,
        final Token token)
    {
        if (null == fieldPrecedenceModel)
        {
            return;
        }

        sb.append("\n")
            .append(indent).append("private void ").append(accessOrderListenerMethodName(token))
            .append("(int remaining, string action)\n")
            .append(indent).append("{\n")
            .append(indent).append(INDENT).append("if (remaining == 0)\n")
            .append(indent).append(INDENT).append("{\n");

        final FieldPrecedenceModel.CodecInteraction selectEmptyGroup =
            fieldPrecedenceModel.interactionFactory().determineGroupIsEmpty(token);

        generateAccessOrderListener(
            sb,
            indent + TWO_INDENT,
            "\" + action + \" count of repeating group",
            fieldPrecedenceModel,
            selectEmptyGroup);

        sb.append(indent).append(INDENT).append("}\n")
            .append(indent).append(INDENT).append("else\n")
            .append(indent).append(INDENT).append("{\n");

        final FieldPrecedenceModel.CodecInteraction selectNonEmptyGroup =
            fieldPrecedenceModel.interactionFactory().determineGroupHasElements(token);

        generateAccessOrderListener(
            sb,
            indent + TWO_INDENT,
            "\" + action + \" count of repeating group",
            fieldPrecedenceModel,
            selectNonEmptyGroup);

        sb.append(indent).append(INDENT).append("}\n")
            .append(indent).append("}\n");
    }

    private static void generateAccessOrderListener(
        final StringBuilder sb,
        final String indent,
        final String action,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final FieldPrecedenceModel.CodecInteraction interaction)
    {
        if (interaction.isTopLevelBlockFieldAccess())
        {
            sb.append(indent).append("if (codecState() == ")
                .append(qualifiedStateCase(fieldPrecedenceModel.notWrappedState()))
                .append(")\n")
                .append(indent).append("{\n");
            generateAccessOrderException(sb, indent + INDENT, action, fieldPrecedenceModel, interaction);
            sb.append(indent).append("}\n");
        }
        else
        {
            sb.append(indent).append("switch (codecState())\n")
                .append(indent).append("{\n");

            fieldPrecedenceModel.forEachTransition(interaction, (transitionGroup) ->
            {
                transitionGroup.forEachStartState((startState) ->
                    sb.append(indent).append(INDENT)
                    .append("case ").append(stateCaseForSwitchCase(startState)).append(":\n"));
                sb.append(indent).append(TWO_INDENT).append("codecState(")
                    .append(qualifiedStateCase(transitionGroup.endState())).append(");\n")
                    .append(indent).append(TWO_INDENT).append("break;\n");
            });

            sb.append(indent).append(INDENT).append("default:\n");
            generateAccessOrderException(sb, indent + TWO_INDENT, action, fieldPrecedenceModel, interaction);
            sb.append(indent).append("}\n");
        }
    }

    private static void generateAccessOrderException(
        final StringBuilder sb,
        final String indent,
        final String action,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final FieldPrecedenceModel.CodecInteraction interaction)
    {
        sb.append(indent).append("throw new InvalidOperationException(")
            .append("\"Illegal field access order. \" +\n")
            .append(indent).append(INDENT)
            .append("\"Cannot ").append(action).append(" \\\"").append(interaction.groupQualifiedName())
            .append("\\\" in state: \" + codecStateName(codecState()) +\n")
            .append(indent).append(INDENT)
            .append("\". Expected one of these transitions: [\" + codecStateTransitions(codecState()) +\n")
            .append(indent).append(INDENT)
            .append("\"]. Please see the diagram in the docs of the enum ")
            .append(fieldPrecedenceModel.generatedRepresentationClassName()).append(".\");\n");
    }

    private static void generateAccessOrderListenerMethodForNextGroupElement(
        final StringBuilder sb,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent,
        final Token token)
    {
        if (null == fieldPrecedenceModel)
        {
            return;
        }

        sb.append("\n");

        sb.append(indent).append("private void OnNextElementAccessed()\n")
            .append(indent).append("{\n")
            .append(indent).append(INDENT).append("int remaining = ").append("_count - _index").append(";\n")
            .append(indent).append(INDENT).append("if (remaining > 1)\n")
            .append(indent).append(INDENT).append("{\n");

        final FieldPrecedenceModel.CodecInteraction selectNextElementInGroup =
            fieldPrecedenceModel.interactionFactory().moveToNextElement(token);

        generateAccessOrderListener(
            sb,
            indent + TWO_INDENT,
            "access next element in repeating group",
            fieldPrecedenceModel,
            selectNextElementInGroup);

        sb.append(indent).append(INDENT).append("}\n")
            .append(indent).append(INDENT).append("else if (remaining == 1)\n")
            .append(indent).append(INDENT).append("{\n");

        final FieldPrecedenceModel.CodecInteraction selectLastElementInGroup =
            fieldPrecedenceModel.interactionFactory().moveToLastElement(token);

        generateAccessOrderListener(
            sb,
            indent + TWO_INDENT,
            "access next element in repeating group",
            fieldPrecedenceModel,
            selectLastElementInGroup);

        sb.append(indent).append(INDENT).append("}\n")
            .append(indent).append("}\n");
    }

    private static void generateAccessOrderListenerMethodForResetGroupCount(
        final StringBuilder sb,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent,
        final Token token)
    {
        if (null == fieldPrecedenceModel)
        {
            return;
        }

        sb.append(indent).append("private void OnResetCountToIndex()\n")
            .append(indent).append("{\n");

        final FieldPrecedenceModel.CodecInteraction resetCountToIndex =
            fieldPrecedenceModel.interactionFactory().resetCountToIndex(token);

        generateAccessOrderListener(
            sb,
            indent + "   ",
            "reset count of repeating group",
            fieldPrecedenceModel,
            resetCountToIndex);

        sb.append(indent).append("}\n");
    }

    private static void generateAccessOrderListenerMethodForVarDataLength(
        final StringBuilder sb,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent,
        final Token token)
    {
        if (null == fieldPrecedenceModel)
        {
            return;
        }

        sb.append("\n")
            .append(indent).append("private void ")
            .append(accessOrderListenerMethodName(token, "Length")).append("()\n")
            .append(indent).append("{\n");

        final FieldPrecedenceModel.CodecInteraction accessLength =
            fieldPrecedenceModel.interactionFactory().accessVarDataLength(token);

        generateAccessOrderListener(
            sb,
            indent + INDENT,
            "decode length of var data",
            fieldPrecedenceModel,
            accessLength);

        sb.append(indent).append("}\n");
    }

    private static CharSequence generateDecoderWrapListener(
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent)
    {
        if (null == fieldPrecedenceModel)
        {
            return "";
        }

        final StringBuilder sb = new StringBuilder();
        sb.append(indent).append("private void OnWrapForDecode(int actingVersion)\n")
            .append(indent).append("{\n")
            .append(indent).append(INDENT).append("switch(actingVersion)\n")
            .append(indent).append(INDENT).append("{\n");

        fieldPrecedenceModel.forEachWrappedStateByVersion((version, state) ->
            sb.append(indent).append(TWO_INDENT).append("case ").append(version).append(":\n")
            .append(indent).append(THREE_INDENT).append("codecState(")
            .append(qualifiedStateCase(state)).append(");\n")
            .append(indent).append(THREE_INDENT).append("break;\n"));

        sb.append(indent).append(TWO_INDENT).append("default:\n")
            .append(indent).append(THREE_INDENT).append("codecState(")
            .append(qualifiedStateCase(fieldPrecedenceModel.latestVersionWrappedState())).append(");\n")
            .append(indent).append(THREE_INDENT).append("break;\n")
            .append(indent).append(INDENT).append("}\n")
            .append(indent).append("}\n\n");

        return sb;
    }

    private CharSequence generateEncoderWrapListener(
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent)
    {
        if (null == fieldPrecedenceModel)
        {
            return "";
        }

        final StringBuilder sb = new StringBuilder();
        sb.append("#if ").append(precedenceChecksFlagName).append("\n")
            .append(indent).append("codecState(")
            .append(qualifiedStateCase(fieldPrecedenceModel.latestVersionWrappedState()))
            .append(");\n")
            .append("#endif\n");
        return sb;
    }

    private CharSequence generateFields(
        final FieldPrecedenceModel fieldPrecedenceModel,
        final List<Token> tokens,
        final String indent)
    {
        final StringBuilder sb = new StringBuilder();

        for (int i = 0, size = tokens.size(); i < size; i++)
        {
            final Token signalToken = tokens.get(i);
            if (signalToken.signal() == Signal.BEGIN_FIELD)
            {
                final Token encodingToken = tokens.get(i + 1);
                final String propertyName = signalToken.name();

                generateFieldIdMethod(sb, signalToken, indent + INDENT);
                generateSinceActingDeprecated(
                    sb, indent + INDENT, CSharpUtil.formatPropertyName(signalToken.name()), signalToken);
                generateOffsetMethod(sb, signalToken, indent + INDENT);
                generateFieldMetaAttributeMethod(sb, signalToken, indent + INDENT);

                generateAccessOrderListenerMethod(sb, fieldPrecedenceModel, indent + INDENT, signalToken);

                switch (encodingToken.signal())
                {
                    case ENCODING:
                        sb.append(generatePrimitiveProperty(propertyName, signalToken, encodingToken,
                            fieldPrecedenceModel, indent));
                        break;

                    case BEGIN_ENUM:
                        sb.append(generateEnumProperty(propertyName, signalToken, encodingToken,
                            fieldPrecedenceModel, indent));
                        break;

                    case BEGIN_SET:
                        sb.append(generateBitSetProperty(propertyName, signalToken, encodingToken,
                            fieldPrecedenceModel, indent));
                        break;

                    case BEGIN_COMPOSITE:
                        sb.append(generateCompositeProperty(propertyName, signalToken, encodingToken,
                            fieldPrecedenceModel, indent));
                        break;

                    default:
                        break;
                }
            }
        }

        return sb;
    }

    private void generateFieldIdMethod(final StringBuilder sb, final Token token, final String indent)
    {
        sb.append(String.format("\n" +
            indent + "public const int %sId = %d;\n",
            CSharpUtil.formatPropertyName(token.name()),
            token.id()));
    }

    private void generateOffsetMethod(final StringBuilder sb, final Token token, final String indent)
    {
        sb.append(String.format("\n" +
            indent + "public const int %sOffset = %d;\n",
            CSharpUtil.formatPropertyName(token.name()),
            token.offset()));
    }

    private void generateFieldMetaAttributeMethod(final StringBuilder sb, final Token token, final String indent)
    {
        final Encoding encoding = token.encoding();
        final String epoch = encoding.epoch() == null ? "" : encoding.epoch();
        final String timeUnit = encoding.timeUnit() == null ? "" : encoding.timeUnit();
        final String semanticType = encoding.semanticType() == null ? "" : encoding.semanticType();
        final String presence = encoding.presence() == null ? "" : encoding.presence().toString().toLowerCase();

        sb.append(String.format("\n" +
            indent + "public static string %sMetaAttribute(MetaAttribute metaAttribute)\n" +
            indent + "{\n" +
            indent + INDENT + "switch (metaAttribute)\n" +
            indent + INDENT + "{\n" +
            indent + INDENT + INDENT + "case MetaAttribute.Epoch: return \"%s\";\n" +
            indent + INDENT + INDENT + "case MetaAttribute.TimeUnit: return \"%s\";\n" +
            indent + INDENT + INDENT + "case MetaAttribute.SemanticType: return \"%s\";\n" +
            indent + INDENT + INDENT + "case MetaAttribute.Presence: return \"%s\";\n" +
            indent + INDENT + "}\n\n" +
            indent + INDENT + "return \"\";\n" +
            indent + "}\n",
            toUpperFirstChar(token.name()),
            epoch,
            timeUnit,
            semanticType,
            presence));
    }

    private CharSequence generateEnumFieldNotPresentCondition(
        final int sinceVersion,
        final String enumName,
        final String indent)
    {
        if (0 == sinceVersion)
        {
            return "";
        }

        return String.format(
            indent + INDENT + INDENT + "if (_actingVersion < %d) return %s.NULL_VALUE;\n\n",
            sinceVersion,
            enumName);
    }

    private CharSequence generateEnumProperty(
        final String propertyName,
        final Token fieldToken,
        final Token typeToken,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent)
    {
        final String enumName = formatClassName(typeToken.applicableTypeName());
        final String typePrefix = toUpperFirstChar(typeToken.encoding().primitiveType().primitiveName());
        final String enumUnderlyingType = cSharpTypeName(typeToken.encoding().primitiveType());
        final int offset = typeToken.offset();
        final ByteOrder byteOrder = typeToken.encoding().byteOrder();
        final String byteOrderStr = generateByteOrder(byteOrder, typeToken.encoding().primitiveType().size());

        if (fieldToken.isConstantEncoding())
        {
            final String constValue = fieldToken.encoding().constValue().toString();

            return String.format("\n" +
                "%1$s" +
                indent + INDENT + "public %2$s %3$s\n" +
                indent + INDENT + "{\n" +
                indent + INDENT + INDENT + "get\n" +
                indent + INDENT + INDENT + "{\n" +
                indent + INDENT + INDENT + INDENT + "return %4$s;\n" +
                indent + INDENT + INDENT + "}\n" +
                indent + INDENT + "}\n\n",
                generateDocumentation(indent + INDENT, fieldToken),
                enumName,
                toUpperFirstChar(propertyName),
                constValue);
        }
        else
        {
            final CharSequence accessOrderListenerCall = generateAccessOrderListenerCall(
                fieldPrecedenceModel, indent + TWO_INDENT, fieldToken);

            return String.format("\n" +
                "%1$s" +
                indent + INDENT + "public %2$s %3$s\n" +
                indent + INDENT + "{\n" +
                indent + INDENT + INDENT + "get\n" +
                indent + INDENT + INDENT + "{\n" +
                "%4$s" +
                "%10$s" +
                indent + INDENT + INDENT + INDENT + "return (%5$s)_buffer.%6$sGet%8$s(_offset + %7$d);\n" +
                indent + INDENT + INDENT + "}\n" +
                indent + INDENT + INDENT + "set\n" +
                indent + INDENT + INDENT + "{\n" +
                "%10$s" +
                indent + INDENT + INDENT + INDENT + "_buffer.%6$sPut%8$s(_offset + %7$d, (%9$s)value);\n" +
                indent + INDENT + INDENT + "}\n" +
                indent + INDENT + "}\n\n",
                generateDocumentation(indent + INDENT, fieldToken),
                enumName,
                toUpperFirstChar(propertyName),
                generateEnumFieldNotPresentCondition(fieldToken.version(), enumName, indent),
                enumName,
                typePrefix,
                offset,
                byteOrderStr,
                enumUnderlyingType,
                accessOrderListenerCall);
        }
    }

    private String generateBitSetProperty(
        final String propertyName,
        final Token fieldToken,
        final Token typeToken,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent)
    {
        final String bitSetName = formatClassName(typeToken.applicableTypeName());
        final int offset = typeToken.offset();
        final String typePrefix = toUpperFirstChar(typeToken.encoding().primitiveType().primitiveName());
        final ByteOrder byteOrder = typeToken.encoding().byteOrder();
        final String byteOrderStr = generateByteOrder(byteOrder, typeToken.encoding().primitiveType().size());
        final String typeName = cSharpTypeName(typeToken.encoding().primitiveType());
        final CharSequence accessOrderListenerCall = generateAccessOrderListenerCall(
            fieldPrecedenceModel, indent + TWO_INDENT, fieldToken);

        return String.format("\n" +
            "%1$s" +
            indent + INDENT + "public %2$s %3$s\n" +
            indent + INDENT + "{\n" +
            indent + INDENT + INDENT + "get\n" +
            indent + INDENT + INDENT + "{\n" +
            "%4$s" +
            "%10$s" +
            indent + INDENT + INDENT + INDENT + "return (%5$s)_buffer.%6$sGet%8$s(_offset + %7$d);\n" +
            indent + INDENT + INDENT + "}\n" +
            indent + INDENT + INDENT + "set\n" +
            indent + INDENT + INDENT + "{\n" +
            "%10$s" +
            indent + INDENT + INDENT + INDENT + "_buffer.%6$sPut%8$s(_offset + %7$d, (%9$s)value);\n" +
            indent + INDENT + INDENT + "}\n" +
            indent + INDENT + "}\n",
            generateDocumentation(indent + INDENT, fieldToken),
            bitSetName,
            toUpperFirstChar(propertyName),
            generateBitSetNotPresentCondition(fieldToken.version(), indent, bitSetName),
            bitSetName,
            typePrefix,
            offset,
            byteOrderStr,
            typeName,
            accessOrderListenerCall);
    }

    private Object generateCompositeProperty(
        final String propertyName,
        final Token fieldToken,
        final Token typeToken,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent)
    {
        final String compositeName = CSharpUtil.formatClassName(typeToken.applicableTypeName());
        final int offset = typeToken.offset();
        final CharSequence accessOrderListenerCall = generateAccessOrderListenerCall(
            fieldPrecedenceModel, indent + THREE_INDENT, fieldToken);
        final StringBuilder sb = new StringBuilder();

        sb.append(String.format("\n" +
            indent + INDENT + "private readonly %1$s _%2$s = new %3$s();\n",
            compositeName,
            toLowerFirstChar(propertyName),
            compositeName));

        sb.append(String.format("\n" +
            "%1$s" +
            indent + INDENT + "public %2$s %3$s\n" +
            indent + INDENT + "{\n" +
            indent + INDENT + INDENT + "get\n" +
            indent + INDENT + INDENT + "{\n" +
            "%4$s" +
            "%7$s" +
            indent + INDENT + INDENT + INDENT + "_%5$s.Wrap(_buffer, _offset + %6$d, _actingVersion);\n" +
            indent + INDENT + INDENT + INDENT + "return _%5$s;\n" +
            indent + INDENT + INDENT + "}\n" +
            indent + INDENT + "}\n",
            generateDocumentation(indent + INDENT, fieldToken),
            compositeName,
            toUpperFirstChar(propertyName),
            generateTypeFieldNotPresentCondition(fieldToken.version(), indent),
            toLowerFirstChar(propertyName),
            offset,
            accessOrderListenerCall));

        return sb;
    }

    private void generateSinceActingDeprecated(
        final StringBuilder sb,
        final String indent,
        final String propertyName,
        final Token token)
    {
        sb.append(String.format(
            indent + "public const int %1$sSinceVersion = %2$d;\n" +
            indent + "public const int %1$sDeprecated = %3$d;\n" +
            indent + "public bool %1$sInActingVersion()\n" +
            indent + "{\n" +
            indent + INDENT + "return _actingVersion >= %1$sSinceVersion;\n" +
            indent + "}\n",
            propertyName,
            token.version(),
            token.deprecated()));
    }

    private String generateByteOrder(final ByteOrder byteOrder, final int primitiveTypeSize)
    {
        if (primitiveTypeSize == 1)
        {
            return "";
        }

        if ("BIG_ENDIAN".equals(byteOrder.toString()))
        {
            return "BigEndian";
        }

        return "LittleEndian";
    }

    private String generateLiteral(final PrimitiveType type, final String value)
    {
        String literal = "";

        final String castType = cSharpTypeName(type);
        switch (type)
        {
            case CHAR:
            case UINT8:
            case INT8:
            case INT16:
            case UINT16:
                literal = "(" + castType + ")" + value;
                break;

            case INT32:
                literal = value;
                break;

            case UINT32:
                literal = value + "U";
                break;

            case FLOAT:
                if (value.endsWith("NaN"))
                {
                    literal = "float.NaN";
                }
                else
                {
                    literal = value + "f";
                }
                break;

            case UINT64:
                literal = "0x" + Long.toHexString(Long.parseLong(value)) + "UL";
                break;

            case INT64:
                literal = value + "L";
                break;

            case DOUBLE:
                if (value.endsWith("NaN"))
                {
                    literal = "double.NaN";
                }
                else
                {
                    literal = value + "d";
                }
                break;
        }

        return literal;
    }

    private void appendGroupInstanceDisplay(
        final StringBuilder sb,
        final List<Token> fields,
        final List<Token> groups,
        final List<Token> varData,
        final String baseIndent)
    {
        final String indent = baseIndent + INDENT;

        sb.append('\n');
        append(sb, indent, "internal void BuildString(StringBuilder builder)");
        append(sb, indent, "{");
        append(sb, indent, "    if (_buffer == null)");
        append(sb, indent, "    {");
        append(sb, indent, "        return;");
        append(sb, indent, "    }");
        sb.append('\n');
        Separators.BEGIN_COMPOSITE.appendToGeneratedBuilder(sb, indent + INDENT);
        appendDisplay(sb, fields, groups, varData, indent + INDENT);
        Separators.END_COMPOSITE.appendToGeneratedBuilder(sb, indent + INDENT);
        sb.append('\n');
        append(sb, indent, "}");
    }

    private void appendDisplay(
        final StringBuilder sb,
        final List<Token> fields,
        final List<Token> groups,
        final List<Token> varData,
        final String indent)
    {
        int lengthBeforeLastGeneratedSeparator = -1;

        for (int i = 0, size = fields.size(); i < size;)
        {
            final Token fieldToken = fields.get(i);
            if (fieldToken.signal() == Signal.BEGIN_FIELD)
            {
                final Token encodingToken = fields.get(i + 1);
                final String fieldName = formatPropertyName(fieldToken.name());
                lengthBeforeLastGeneratedSeparator = writeTokenDisplay(fieldName, encodingToken, sb, indent);

                i += fieldToken.componentTokenCount();
            }
            else
            {
                i++;
            }
        }

        for (int i = 0, size = groups.size(); i < size; i++)
        {
            final Token groupToken = groups.get(i);
            if (groupToken.signal() != Signal.BEGIN_GROUP)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_GROUP: token=" + groupToken);
            }

            final String groupName = formatPropertyName(groupToken.name());
            final String varName = formatVariableName(groupToken.name());

            append(
                sb, indent, "builder.Append(\"" + groupName + Separators.KEY_VALUE + Separators.BEGIN_GROUP + "\");");
            append(sb, indent, "var " + varName + " = this." + groupName + ";");
            append(sb, indent, "if (" + varName + ".Count > 0)");
            append(sb, indent, "{");
            append(sb, indent, "    var first = true;");
            append(sb, indent, "    while (" + varName + ".HasNext)");
            append(sb, indent, "    {");
            append(sb, indent, "        if (!first)");
            append(sb, indent, "        {");
            append(sb, indent, "            builder.Append(',');");
            append(sb, indent, "        }");
            append(sb, indent, "        first = false;");
            append(sb, indent, "        " + varName + ".Next().BuildString(builder);");
            append(sb, indent, "    }");
            append(sb, indent, "}");
            append(sb, indent, "builder.Append(\"" + Separators.END_GROUP + "\");");

            lengthBeforeLastGeneratedSeparator = sb.length();
            Separators.FIELD.appendToGeneratedBuilder(sb, indent);

            i = findEndSignal(groups, i, Signal.END_GROUP, groupToken.name());
        }

        for (int i = 0, size = varData.size(); i < size;)
        {
            final Token lengthToken = Generators.findFirst("length", varData, i);
            final int sizeOfLengthField = lengthToken.encodedLength();
            final Token varDataToken = varData.get(i);
            if (varDataToken.signal() != Signal.BEGIN_VAR_DATA)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_VAR_DATA: token=" + varDataToken);
            }

            final String characterEncoding = varData.get(i + 3).encoding().characterEncoding();
            final String varDataName = formatPropertyName(varDataToken.name());
            final String getterName = formatGetterName(varDataToken.name());
            append(sb, indent, "builder.Append(\"" + varDataName + Separators.KEY_VALUE + "\");");
            if (characterEncoding == null)
            {
                final String name = Generators.toUpperFirstChar(varDataToken.name());
                append(sb, indent, "builder.Append(" + name + "Length()).Append(\" bytes of raw data\");");
                append(sb, indent, "_parentMessage.Limit = _parentMessage.Limit + " +
                    sizeOfLengthField + " + " + name + "Length();\n");
            }
            else
            {
                append(sb, indent, "builder.Append('\\'').Append(" + getterName + "()).Append('\\'');");
            }

            lengthBeforeLastGeneratedSeparator = sb.length();
            Separators.FIELD.appendToGeneratedBuilder(sb, indent);

            i += varDataToken.componentTokenCount();
        }

        if (lengthBeforeLastGeneratedSeparator > -1)
        {
            sb.setLength(lengthBeforeLastGeneratedSeparator);
        }
    }

    private int writeTokenDisplay(
        final String fieldName,
        final Token typeToken,
        final StringBuilder sb,
        final String indent)
    {
        if (typeToken.encodedLength() <= 0)
        {
            return -1;
        }

        append(sb, indent, "builder.Append(\"" + fieldName + Separators.KEY_VALUE + "\");");

        if (typeToken.isConstantEncoding())
        {
            append(sb, indent, "builder.Append(\"" + typeToken.encoding().constValue() + "\");");
        }
        else
        {
            switch (typeToken.signal())
            {
                case ENCODING:
                    if (typeToken.arrayLength() > 1)
                    {
                        if (typeToken.encoding().primitiveType() == PrimitiveType.CHAR)
                        {
                            append(sb, indent, "builder.Append(\"'\");");
                            append(sb, indent, "for (int i = 0; i < " + fieldName +
                                "Length && this.Get" + fieldName + "(i) > 0; ++i)");
                            append(sb, indent, "{");
                            append(sb, indent, "    builder.Append((char)this.Get" + fieldName + "(i));");
                            append(sb, indent, "}");
                            append(sb, indent, "builder.Append(\"'\");");
                        }
                        else
                        {
                            Separators.BEGIN_ARRAY.appendToGeneratedBuilder(sb, indent);
                            append(sb, indent, "for (int i = 0; i < " + fieldName + "Length; ++i)");
                            append(sb, indent, "{");
                            append(sb, indent, "    if (i > 0)");
                            append(sb, indent, "    {");
                            append(sb, indent, "        builder.Append(',');");
                            append(sb, indent, "    }");
                            append(sb, indent, "    builder.Append(Get" + fieldName + "(i));");
                            append(sb, indent, "}");
                            Separators.END_ARRAY.appendToGeneratedBuilder(sb, indent);
                        }
                    }
                    else
                    {
                        append(sb, indent, "builder.Append(this." + fieldName + ");");
                    }
                    break;

                case BEGIN_ENUM:
                    append(sb, indent, "builder.Append(this." + fieldName + ");");
                    break;

                case BEGIN_SET:
                    append(sb, indent, "this." + fieldName + ".BuildString(builder);");
                    break;

                case BEGIN_COMPOSITE:
                    append(sb, indent, "if (this." + fieldName + " != null)");
                    append(sb, indent, "{");
                    append(sb, indent, "    this." + fieldName + ".BuildString(builder);");
                    append(sb, indent, "}");
                    append(sb, indent, "else");
                    append(sb, indent, "{");
                    append(sb, indent, "    builder.Append(\"null\");");
                    append(sb, indent, "}");
                    break;

                default:
                    break;
            }
        }

        final int lengthBeforeFieldSeparator = sb.length();
        Separators.FIELD.appendToGeneratedBuilder(sb, indent);

        return lengthBeforeFieldSeparator;
    }

    private void appendToString(final StringBuilder sb, final String indent)
    {
        sb.append('\n');
        append(sb, indent, "public override string ToString()");
        append(sb, indent, "{");
        append(sb, indent, "    var sb = new StringBuilder(100);");
        append(sb, indent, "    this.BuildString(sb);");
        append(sb, indent, "    return sb.ToString();");
        append(sb, indent, "}");
    }

    private CharSequence generateChoiceDisplay(final String enumName)
    {
        final StringBuilder sb = new StringBuilder();

        sb.append('\n');
        append(sb, INDENT, "static class " + enumName + "Ext");
        append(sb, INDENT, "{");
        append(sb, TWO_INDENT, "internal static void BuildString(this " + enumName + " val, StringBuilder builder)");
        append(sb, TWO_INDENT, "{");
        Separators.BEGIN_SET.appendToGeneratedBuilder(sb, THREE_INDENT);
        append(sb, THREE_INDENT, "builder.Append(val);");
        Separators.END_SET.appendToGeneratedBuilder(sb, THREE_INDENT);
        append(sb, TWO_INDENT, "}");
        append(sb, INDENT, "}");

        return sb;
    }

    private CharSequence generateDisplay(
        final String name,
        final List<Token> tokens,
        final List<Token> groups,
        final List<Token> varData,
        final FieldPrecedenceModel fieldPrecedenceModel)
    {
        final StringBuilder sb = new StringBuilder(100);

        appendToString(sb, TWO_INDENT);
        sb.append('\n');
        append(sb, TWO_INDENT, "internal void BuildString(StringBuilder builder)");
        append(sb, TWO_INDENT, "{");
        append(sb, TWO_INDENT, "    if (_buffer == null)");
        append(sb, TWO_INDENT, "    {");
        append(sb, TWO_INDENT, "        throw new ArgumentNullException(\"_buffer\");");
        append(sb, TWO_INDENT, "    }");
        sb.append('\n');
        append(sb, TWO_INDENT, "    int originalLimit = this.Limit;");
        if (null != fieldPrecedenceModel)
        {
            sb.append("#if ").append(precedenceChecksFlagName).append("\n");
            append(sb, TWO_INDENT, "    CodecState originalState = _codecState;");
            sb.append(THREE_INDENT).append("_codecState = ")
                .append(qualifiedStateCase(fieldPrecedenceModel.notWrappedState())).append(";\n");
            append(sb, TWO_INDENT, "    OnWrapForDecode(_actingVersion);");
            sb.append("#endif\n");
        }
        append(sb, TWO_INDENT, "    this.Limit = _offset + _actingBlockLength;");
        append(sb, TWO_INDENT, "    builder.Append(\"[" + name + "](sbeTemplateId=\");");
        append(sb, TWO_INDENT, "    builder.Append(" + name + ".TemplateId);");
        append(sb, TWO_INDENT, "    builder.Append(\"|sbeSchemaId=\");");
        append(sb, TWO_INDENT, "    builder.Append(" + name + ".SchemaId);");
        append(sb, TWO_INDENT, "    builder.Append(\"|sbeSchemaVersion=\");");
        append(sb, TWO_INDENT, "    if (_parentMessage._actingVersion != " + name + ".SchemaVersion)");
        append(sb, TWO_INDENT, "    {");
        append(sb, TWO_INDENT, "        builder.Append(_parentMessage._actingVersion);");
        append(sb, TWO_INDENT, "        builder.Append('/');");
        append(sb, TWO_INDENT, "    }");
        append(sb, TWO_INDENT, "    builder.Append(" + name + ".SchemaVersion);");
        append(sb, TWO_INDENT, "    builder.Append(\"|sbeBlockLength=\");");
        append(sb, TWO_INDENT, "    if (_actingBlockLength != " + name + ".BlockLength)");
        append(sb, TWO_INDENT, "    {");
        append(sb, TWO_INDENT, "        builder.Append(_actingBlockLength);");
        append(sb, TWO_INDENT, "        builder.Append('/');");
        append(sb, TWO_INDENT, "    }");
        append(sb, TWO_INDENT, "    builder.Append(" + name + ".BlockLength);");
        append(sb, TWO_INDENT, "    builder.Append(\"):\");");
        sb.append('\n');
        appendDisplay(sb, tokens, groups, varData, THREE_INDENT);
        sb.append('\n');
        if (null != fieldPrecedenceModel)
        {
            sb.append("#if ").append(precedenceChecksFlagName).append("\n");
            append(sb, TWO_INDENT, "    _codecState = originalState;");
            sb.append("#endif\n");
        }
        append(sb, TWO_INDENT, "    this.Limit = originalLimit;");
        sb.append('\n');
        append(sb, TWO_INDENT, "}");
        return sb;
    }

    private CharSequence generateCompositeDisplay(final List<Token> tokens)
    {
        final StringBuilder sb = new StringBuilder();

        appendToString(sb, TWO_INDENT);
        sb.append('\n');
        append(sb, TWO_INDENT, "internal void BuildString(StringBuilder builder)");
        append(sb, TWO_INDENT, "{");
        append(sb, TWO_INDENT, "    if (_buffer == null)");
        append(sb, TWO_INDENT, "    {");
        append(sb, TWO_INDENT, "        return;");
        append(sb, TWO_INDENT, "    }");
        sb.append('\n');
        Separators.BEGIN_COMPOSITE.appendToGeneratedBuilder(sb, THREE_INDENT);

        int lengthBeforeLastGeneratedSeparator = -1;

        for (int i = 1, end = tokens.size() - 1; i < end;)
        {
            final Token encodingToken = tokens.get(i);
            final String propertyName = formatPropertyName(encodingToken.name());
            lengthBeforeLastGeneratedSeparator = writeTokenDisplay(propertyName, encodingToken, sb, THREE_INDENT);
            i += encodingToken.componentTokenCount();
        }

        if (lengthBeforeLastGeneratedSeparator > -1)
        {
            sb.setLength(lengthBeforeLastGeneratedSeparator);
        }

        Separators.END_COMPOSITE.appendToGeneratedBuilder(sb, THREE_INDENT);
        sb.append('\n');
        append(sb, TWO_INDENT, "}");

        return sb;
    }
}
