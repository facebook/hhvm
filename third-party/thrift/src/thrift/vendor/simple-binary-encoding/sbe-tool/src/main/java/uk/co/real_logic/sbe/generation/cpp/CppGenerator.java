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
package uk.co.real_logic.sbe.generation.cpp;

import uk.co.real_logic.sbe.PrimitiveType;
import uk.co.real_logic.sbe.generation.CodeGenerator;
import uk.co.real_logic.sbe.generation.Generators;
import uk.co.real_logic.sbe.generation.common.FieldPrecedenceModel;
import uk.co.real_logic.sbe.generation.common.PrecedenceChecks;
import uk.co.real_logic.sbe.ir.Encoding;
import uk.co.real_logic.sbe.ir.Ir;
import uk.co.real_logic.sbe.ir.Signal;
import uk.co.real_logic.sbe.ir.Token;
import org.agrona.Strings;
import org.agrona.Verify;
import org.agrona.collections.MutableBoolean;
import org.agrona.generation.OutputManager;

import java.io.IOException;
import java.io.Writer;
import java.nio.ByteOrder;
import java.util.*;

import static uk.co.real_logic.sbe.generation.Generators.toLowerFirstChar;
import static uk.co.real_logic.sbe.generation.Generators.toUpperFirstChar;
import static uk.co.real_logic.sbe.generation.cpp.CppUtil.*;
import static uk.co.real_logic.sbe.ir.GenerationUtil.*;

/**
 * Codec generator for the C++11 programming language with conditional compilation for additional C++14 and C++17
 * features.
 */
@SuppressWarnings("MethodLength")
public class CppGenerator implements CodeGenerator
{
    private static final boolean DISABLE_IMPLICIT_COPYING = Boolean.parseBoolean(
        System.getProperty("sbe.cpp.disable.implicit.copying", "false"));
    private static final String BASE_INDENT = "";
    private static final String INDENT = "    ";
    private static final String TWO_INDENT = INDENT + INDENT;
    private static final String THREE_INDENT = TWO_INDENT + INDENT;

    private final Ir ir;
    private final OutputManager outputManager;
    private final boolean shouldDecodeUnknownEnumValues;
    private final PrecedenceChecks precedenceChecks;
    private final String precedenceChecksFlagName;

    /**
     * Create a new Cpp language {@link CodeGenerator}.
     *
     * @param ir                            for the messages and types.
     * @param shouldDecodeUnknownEnumValues generate support for unknown enum values when decoding.
     * @param outputManager                 for generating the codecs to.
     */
    public CppGenerator(final Ir ir, final boolean shouldDecodeUnknownEnumValues, final OutputManager outputManager)
    {
        this(
            ir,
            shouldDecodeUnknownEnumValues,
            PrecedenceChecks.newInstance(new PrecedenceChecks.Context()),
            outputManager
        );
    }

    /**
     * Create a new Go language {@link CodeGenerator}.
     *
     * @param ir                            for the messages and types.
     * @param shouldDecodeUnknownEnumValues generate support for unknown enum values when decoding.
     * @param precedenceChecks              whether and how to generate field precedence checks.
     * @param outputManager                 for generating the codecs to.
     */
    public CppGenerator(
        final Ir ir,
        final boolean shouldDecodeUnknownEnumValues,
        final PrecedenceChecks precedenceChecks,
        final OutputManager outputManager)
    {
        Verify.notNull(ir, "ir");
        Verify.notNull(outputManager, "outputManager");

        this.ir = ir;
        this.shouldDecodeUnknownEnumValues = shouldDecodeUnknownEnumValues;
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

    private List<String> generateTypeStubs() throws IOException
    {
        final List<String> typesToInclude = new ArrayList<>();

        for (final List<Token> tokens : ir.types())
        {
            switch (tokens.get(0).signal())
            {
                case BEGIN_ENUM:
                    generateEnum(tokens);
                    break;

                case BEGIN_SET:
                    generateChoiceSet(tokens);
                    break;

                case BEGIN_COMPOSITE:
                    generateComposite(tokens);
                    break;

                default:
                    break;
            }

            typesToInclude.add(tokens.get(0).applicableTypeName());
        }

        return typesToInclude;
    }

    private List<String> generateTypesToIncludes(final List<Token> tokens)
    {
        final List<String> typesToInclude = new ArrayList<>();

        for (final Token token : tokens)
        {
            switch (token.signal())
            {
                case BEGIN_ENUM:
                case BEGIN_SET:
                case BEGIN_COMPOSITE:
                    typesToInclude.add(token.applicableTypeName());
                    break;

                default:
                    break;
            }
        }

        return typesToInclude;
    }

    /**
     * {@inheritDoc}
     */
    public void generate() throws IOException
    {
        generateMessageHeaderStub();
        final List<String> typesToInclude = generateTypeStubs();

        for (final List<Token> tokens : ir.messages())
        {
            final Token msgToken = tokens.get(0);
            final String className = formatClassName(msgToken.name());
            final String stateClassName = className + "::CodecState";
            final FieldPrecedenceModel fieldPrecedenceModel = precedenceChecks.createCodecModel(stateClassName, tokens);

            try (Writer out = outputManager.createOutput(className))
            {

                final List<Token> messageBody = tokens.subList(1, tokens.size() - 1);
                int i = 0;

                final List<Token> fields = new ArrayList<>();
                i = collectFields(messageBody, i, fields);

                final List<Token> groups = new ArrayList<>();
                i = collectGroups(messageBody, i, groups);

                final List<Token> varData = new ArrayList<>();
                collectVarData(messageBody, i, varData);

                out.append(generateFileHeader(ir.namespaces(), className, typesToInclude));
                out.append(generateClassDeclaration(className));
                out.append(generateMessageFlyweightCode(className, msgToken, fieldPrecedenceModel));
                out.append(generateFullyEncodedCheck(fieldPrecedenceModel));

                final StringBuilder sb = new StringBuilder();
                generateFields(sb, className, fields, fieldPrecedenceModel, BASE_INDENT);
                generateGroups(sb, groups, fieldPrecedenceModel, BASE_INDENT);
                generateVarData(sb, className, varData, fieldPrecedenceModel, BASE_INDENT);
                generateDisplay(sb, msgToken.name(), fields, groups, varData);
                sb.append(generateMessageLength(groups, varData, BASE_INDENT));
                sb.append("};\n");
                generateLookupTableDefinitions(sb, className, fieldPrecedenceModel);
                sb.append(CppUtil.closingBraces(ir.namespaces().length)).append("#endif\n");
                out.append(sb);
            }
        }
    }

    private CharSequence generateFullyEncodedCheck(final FieldPrecedenceModel fieldPrecedenceModel)
    {
        if (null == fieldPrecedenceModel)
        {
            return "";
        }

        final String indent = "    ";
        final StringBuilder sb = new StringBuilder();
        sb.append("\n");

        sb.append(indent).append("void checkEncodingIsComplete()\n")
            .append(indent).append("{\n")
            .append("#if defined(").append(precedenceChecksFlagName).append(")\n")
            .append(indent).append(INDENT).append("switch (m_codecState)\n")
            .append(indent).append(INDENT).append("{\n");

        fieldPrecedenceModel.forEachTerminalEncoderState((state) ->
            sb.append(indent).append(TWO_INDENT).append("case ").append(stateCaseForSwitchCase(state)).append(":\n")
            .append(indent).append(THREE_INDENT).append("return;\n"));

        sb.append(indent).append(TWO_INDENT).append("default:\n")
            .append(indent).append(THREE_INDENT)
            .append("throw AccessOrderError(std::string(\"Not fully encoded, current state: \") +\n")
            .append(indent).append(THREE_INDENT)
            .append(INDENT).append("codecStateName(m_codecState) + \", allowed transitions: \" +\n")
            .append(indent).append(THREE_INDENT)
            .append(INDENT).append("codecStateTransitions(m_codecState));\n")
            .append(indent).append(INDENT).append("}\n")
            .append("#endif\n");

        sb.append(indent).append("}\n\n");

        return sb;
    }

    private static String accessOrderListenerMethodName(final Token token)
    {
        return "on" + Generators.toUpperFirstChar(token.name()) + "Accessed";
    }

    private static String accessOrderListenerMethodName(final Token token, final String suffix)
    {
        return "on" + Generators.toUpperFirstChar(token.name()) + suffix + "Accessed";
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

        final FieldPrecedenceModel.CodecInteraction fieldAccess =
            fieldPrecedenceModel.interactionFactory().accessField(token);

        final String constDeclaration = canChangeState(fieldPrecedenceModel, fieldAccess) ? "" : " const";

        sb.append("\n")
            .append(indent).append("private:\n")
            .append(indent).append(INDENT).append("void ").append(accessOrderListenerMethodName(token)).append("()")
            .append(constDeclaration).append("\n")
            .append(indent).append(INDENT).append("{\n");

        generateAccessOrderListener(
            sb,
            indent + TWO_INDENT,
            "access field",
            fieldPrecedenceModel,
            fieldAccess);

        sb.append(indent).append(INDENT).append("}\n\n")
            .append(indent).append("public:");
    }

    private static boolean canChangeState(
        final FieldPrecedenceModel fieldPrecedenceModel,
        final FieldPrecedenceModel.CodecInteraction fieldAccess)
    {
        if (fieldAccess.isTopLevelBlockFieldAccess())
        {
            return false;
        }

        final MutableBoolean canChangeState = new MutableBoolean(false);
        fieldPrecedenceModel.forEachTransition(fieldAccess, transition ->
        {
            if (!transition.alwaysEndsInStartState())
            {
                canChangeState.set(true);
            }
        });

        return canChangeState.get();
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
        sb.append("#if defined(").append(precedenceChecksFlagName).append(")\n")
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
            .append(indent).append("private:\n")
            .append(indent).append(INDENT).append("void ").append(accessOrderListenerMethodName(token))
            .append("(std::uint64_t remaining, std::string action)\n")
            .append(indent).append(INDENT).append("{\n")
            .append(indent).append(TWO_INDENT).append("if (0 == remaining)\n")
            .append(indent).append(TWO_INDENT).append("{\n");

        final FieldPrecedenceModel.CodecInteraction selectEmptyGroup =
            fieldPrecedenceModel.interactionFactory().determineGroupIsEmpty(token);

        generateAccessOrderListener(
            sb,
            indent + THREE_INDENT,
            "\" + action + \" count of repeating group",
            fieldPrecedenceModel,
            selectEmptyGroup);

        sb.append(indent).append(TWO_INDENT).append("}\n")
            .append(indent).append(TWO_INDENT).append("else\n")
            .append(indent).append(TWO_INDENT).append("{\n");

        final FieldPrecedenceModel.CodecInteraction selectNonEmptyGroup =
            fieldPrecedenceModel.interactionFactory().determineGroupHasElements(token);

        generateAccessOrderListener(
            sb,
            indent + THREE_INDENT,
            "\" + action + \" count of repeating group",
            fieldPrecedenceModel,
            selectNonEmptyGroup);

        sb.append(indent).append(TWO_INDENT).append("}\n")
            .append(indent).append(INDENT).append("}\n\n")
            .append(indent).append("public:");
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
            .append(indent).append("private:\n")
            .append(indent).append(INDENT).append("void ").append(accessOrderListenerMethodName(token, "Length"))
            .append("() const\n")
            .append(indent).append(INDENT).append("{\n");

        final FieldPrecedenceModel.CodecInteraction accessLength =
            fieldPrecedenceModel.interactionFactory().accessVarDataLength(token);

        generateAccessOrderListener(
            sb,
            indent + TWO_INDENT,
            "decode length of var data",
            fieldPrecedenceModel,
            accessLength);

        sb.append(indent).append(INDENT).append("}\n\n")
            .append(indent).append("public:");
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

                if (!transitionGroup.alwaysEndsInStartState())
                {
                    sb.append(indent).append(TWO_INDENT).append("codecState(")
                        .append(qualifiedStateCase(transitionGroup.endState())).append(");\n");
                }

                sb.append(indent).append(TWO_INDENT).append("break;\n");
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
        sb.append(indent).append("throw AccessOrderError(")
            .append("std::string(\"Illegal field access order. \") +\n")
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

        sb.append(indent).append(INDENT).append("void onNextElementAccessed()\n")
            .append(indent).append(INDENT).append("{\n")
            .append(indent).append(TWO_INDENT).append("std::uint64_t remaining = m_count - m_index;\n")
            .append(indent).append(TWO_INDENT).append("if (remaining > 1)\n")
            .append(indent).append(TWO_INDENT).append("{\n");

        final FieldPrecedenceModel.CodecInteraction selectNextElementInGroup =
            fieldPrecedenceModel.interactionFactory().moveToNextElement(token);

        generateAccessOrderListener(
            sb,
            indent + THREE_INDENT,
            "access next element in repeating group",
            fieldPrecedenceModel,
            selectNextElementInGroup);

        sb.append(indent).append(TWO_INDENT).append("}\n")
            .append(indent).append(TWO_INDENT).append("else if (1 == remaining)\n")
            .append(indent).append(TWO_INDENT).append("{\n");

        final FieldPrecedenceModel.CodecInteraction selectLastElementInGroup =
            fieldPrecedenceModel.interactionFactory().moveToLastElement(token);

        generateAccessOrderListener(
            sb,
            indent + THREE_INDENT,
            "access next element in repeating group",
            fieldPrecedenceModel,
            selectLastElementInGroup);

        sb.append(indent).append(TWO_INDENT).append("}\n")
            .append(indent).append(INDENT).append("}\n");
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

        sb.append("\n")
            .append(indent).append(INDENT).append("void onResetCountToIndex()\n")
            .append(indent).append(INDENT).append("{\n");

        final FieldPrecedenceModel.CodecInteraction resetCountToIndex =
            fieldPrecedenceModel.interactionFactory().resetCountToIndex(token);

        generateAccessOrderListener(
            sb,
            indent + TWO_INDENT,
            "reset count of repeating group",
            fieldPrecedenceModel,
            resetCountToIndex);

        sb.append(indent).append(INDENT).append("}\n");
    }

    private void generateGroups(
        final StringBuilder sb,
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
            final Token numInGroupToken = Generators.findFirst("numInGroup", tokens, i);
            final String cppTypeForNumInGroup = cppTypeName(numInGroupToken.encoding().primitiveType());

            generateGroupClassHeader(sb, groupName, groupToken, tokens, fieldPrecedenceModel, i, indent + INDENT);

            ++i;
            final int groupHeaderTokenCount = tokens.get(i).componentTokenCount();
            i += groupHeaderTokenCount;

            final List<Token> fields = new ArrayList<>();
            i = collectFields(tokens, i, fields);
            generateFields(sb, formatClassName(groupName), fields, fieldPrecedenceModel, indent + INDENT);

            final List<Token> groups = new ArrayList<>();
            i = collectGroups(tokens, i, groups);
            generateGroups(sb, groups, fieldPrecedenceModel, indent + INDENT);

            final List<Token> varData = new ArrayList<>();
            i = collectVarData(tokens, i, varData);
            generateVarData(sb, formatClassName(groupName), varData, fieldPrecedenceModel, indent + INDENT);

            sb.append(generateGroupDisplay(groupName, fields, groups, varData, indent + INDENT + INDENT));
            sb.append(generateMessageLength(groups, varData, indent + INDENT + INDENT));

            sb.append(indent).append("    };\n");
            generateGroupProperty(sb, groupName, groupToken, cppTypeForNumInGroup, fieldPrecedenceModel, indent);
        }
    }

    private void generateGroupClassHeader(
        final StringBuilder sb,
        final String groupName,
        final Token groupToken,
        final List<Token> tokens,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final int index,
        final String indent)
    {
        final String dimensionsClassName = formatClassName(tokens.get(index + 1).name());
        final int dimensionHeaderLength = tokens.get(index + 1).encodedLength();
        final int blockLength = tokens.get(index).encodedLength();
        final Token blockLengthToken = Generators.findFirst("blockLength", tokens, index);
        final Token numInGroupToken = Generators.findFirst("numInGroup", tokens, index);
        final String cppTypeBlockLength = cppTypeName(blockLengthToken.encoding().primitiveType());
        final String cppTypeNumInGroup = cppTypeName(numInGroupToken.encoding().primitiveType());

        final String groupClassName = formatClassName(groupName);

        new Formatter(sb).format("\n" +
            indent + "class %1$s\n" +
            indent + "{\n" +
            indent + "private:\n" +
            indent + "    char *m_buffer = nullptr;\n" +
            indent + "    std::uint64_t m_bufferLength = 0;\n" +
            indent + "    std::uint64_t m_initialPosition = 0;\n" +
            indent + "    std::uint64_t *m_positionPtr = nullptr;\n" +
            indent + "    std::uint64_t m_blockLength = 0;\n" +
            indent + "    std::uint64_t m_count = 0;\n" +
            indent + "    std::uint64_t m_index = 0;\n" +
            indent + "    std::uint64_t m_offset = 0;\n" +
            indent + "    std::uint64_t m_actingVersion = 0;\n\n" +

            indent + "    SBE_NODISCARD std::uint64_t *sbePositionPtr() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return m_positionPtr;\n" +
            indent + "    }\n\n",
            groupClassName);

        if (null != fieldPrecedenceModel)
        {
            new Formatter(sb).format(
                indent + "    CodecState *m_codecStatePtr = nullptr;\n\n" +

                indent + "    CodecState codecState() const SBE_NOEXCEPT\n" +
                indent + "    {\n" +
                indent + "        return *m_codecStatePtr;\n" +
                indent + "    }\n\n" +

                indent + "    CodecState *codecStatePtr()\n" +
                indent + "    {\n" +
                indent + "        return m_codecStatePtr;\n" +
                indent + "    }\n\n" +

                indent + "    void codecState(CodecState codecState)\n" +
                indent + "    {\n" +
                indent + "        *m_codecStatePtr = codecState;\n" +
                indent + "    }\n\n"
            );
        }

        sb.append(generateHiddenCopyConstructor(indent + "    ", groupClassName));

        final String codecStateParameter = null == fieldPrecedenceModel ?
            ")\n" :
            ",\n " + indent + "        CodecState *codecState)\n";

        final String codecStateAssignment = null == fieldPrecedenceModel ?
            "" :
            indent + "        m_codecStatePtr = codecState;\n";

        new Formatter(sb).format(
            indent + "public:\n" +
            indent + "    %5$s() = default;\n\n" +

            indent + "    inline void wrapForDecode(\n" +
            indent + "        char *buffer,\n" +
            indent + "        std::uint64_t *pos,\n" +
            indent + "        const std::uint64_t actingVersion,\n" +
            indent + "        const std::uint64_t bufferLength%3$s" +
            indent + "    {\n" +
            indent + "        %2$s dimensions(buffer, *pos, bufferLength, actingVersion);\n" +
            indent + "        m_buffer = buffer;\n" +
            indent + "        m_bufferLength = bufferLength;\n" +
            indent + "        m_blockLength = dimensions.blockLength();\n" +
            indent + "        m_count = dimensions.numInGroup();\n" +
            indent + "        m_index = 0;\n" +
            indent + "        m_actingVersion = actingVersion;\n" +
            indent + "        m_initialPosition = *pos;\n" +
            indent + "        m_positionPtr = pos;\n" +
            indent + "        *m_positionPtr = *m_positionPtr + %1$d;\n" +
            "%4$s" +
            indent + "    }\n",
            dimensionHeaderLength,
            dimensionsClassName,
            codecStateParameter,
            codecStateAssignment,
            groupClassName);

        final long minCount = numInGroupToken.encoding().applicableMinValue().longValue();
        final String minCheck = minCount > 0 ? "count < " + minCount + " || " : "";

        new Formatter(sb).format("\n" +
            indent + "    inline void wrapForEncode(\n" +
            indent + "        char *buffer,\n" +
            indent + "        const %3$s count,\n" +
            indent + "        std::uint64_t *pos,\n" +
            indent + "        const std::uint64_t actingVersion,\n" +
            indent + "        const std::uint64_t bufferLength%8$s" +
            indent + "    {\n" +
            indent + "#if defined(__GNUG__) && !defined(__clang__)\n" +
            indent + "#pragma GCC diagnostic push\n" +
            indent + "#pragma GCC diagnostic ignored \"-Wtype-limits\"\n" +
            indent + "#endif\n" +
            indent + "        if (%5$scount > %6$d)\n" +
            indent + "        {\n" +
            indent + "            throw std::runtime_error(\"count outside of allowed range [E110]\");\n" +
            indent + "        }\n" +
            indent + "#if defined(__GNUG__) && !defined(__clang__)\n" +
            indent + "#pragma GCC diagnostic pop\n" +
            indent + "#endif\n" +
            indent + "        m_buffer = buffer;\n" +
            indent + "        m_bufferLength = bufferLength;\n" +
            indent + "        %7$s dimensions(buffer, *pos, bufferLength, actingVersion);\n" +
            indent + "        dimensions.blockLength(static_cast<%1$s>(%2$d));\n" +
            indent + "        dimensions.numInGroup(static_cast<%3$s>(count));\n" +
            indent + "        m_index = 0;\n" +
            indent + "        m_count = count;\n" +
            indent + "        m_blockLength = %2$d;\n" +
            indent + "        m_actingVersion = actingVersion;\n" +
            indent + "        m_initialPosition = *pos;\n" +
            indent + "        m_positionPtr = pos;\n" +
            indent + "        *m_positionPtr = *m_positionPtr + %4$d;\n" +
            "%9$s" +
            indent + "    }\n",
            cppTypeBlockLength,
            blockLength,
            cppTypeNumInGroup,
            dimensionHeaderLength,
            minCheck,
            numInGroupToken.encoding().applicableMaxValue().longValue(),
            dimensionsClassName,
            codecStateParameter,
            codecStateAssignment);

        if (groupToken.version() > 0)
        {
            final String codecStateNullAssignment = null == fieldPrecedenceModel ?
                "" :
                indent + "        m_codecStatePtr = nullptr;\n";

            new Formatter(sb).format(
                indent + "    inline void notPresent(std::uint64_t actingVersion)\n" +
                indent + "    {\n" +
                indent + "        m_buffer = nullptr;\n" +
                indent + "        m_bufferLength = 0;\n" +
                indent + "        m_blockLength = 0;\n" +
                indent + "        m_count = 0;\n" +
                indent + "        m_index = 0;\n" +
                indent + "        m_actingVersion = actingVersion;\n" +
                indent + "        m_initialPosition = 0;\n" +
                indent + "        m_positionPtr = nullptr;\n" +
                "%1$s" +
                indent + "    }\n",
                codecStateNullAssignment);
        }

        if (null != fieldPrecedenceModel)
        {
            sb.append("\n").append(indent).append("private:");
            generateAccessOrderListenerMethodForNextGroupElement(sb, fieldPrecedenceModel, indent, groupToken);
            generateAccessOrderListenerMethodForResetGroupCount(sb, fieldPrecedenceModel, indent, groupToken);
            sb.append("\n").append(indent).append("public:");
        }

        final CharSequence onNextAccessOrderCall = null == fieldPrecedenceModel ? "" :
            generateAccessOrderListenerCall(fieldPrecedenceModel, indent + TWO_INDENT, "onNextElementAccessed");

        new Formatter(sb).format("\n" +
            indent + "    static SBE_CONSTEXPR std::uint64_t sbeHeaderSize() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %1$d;\n" +
            indent + "    }\n\n" +

            indent + "    static SBE_CONSTEXPR std::uint64_t sbeBlockLength() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %2$d;\n" +
            indent + "    }\n\n" +

            indent + "    SBE_NODISCARD std::uint64_t sbeActingBlockLength() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return m_blockLength;\n" +
            indent + "    }\n\n" +

            indent + "    SBE_NODISCARD std::uint64_t sbePosition() const SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return *m_positionPtr;\n" +
            indent + "    }\n\n" +

            indent + "    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)\n" +
            indent + "    std::uint64_t sbeCheckPosition(const std::uint64_t position)\n" +
            indent + "    {\n" +
            indent + "        if (SBE_BOUNDS_CHECK_EXPECT((position > m_bufferLength), false))\n" +
            indent + "        {\n" +
            indent + "            throw std::runtime_error(\"buffer too short [E100]\");\n" +
            indent + "        }\n" +
            indent + "        return position;\n" +
            indent + "    }\n\n" +

            indent + "    void sbePosition(const std::uint64_t position)\n" +
            indent + "    {\n" +
            indent + "        *m_positionPtr = sbeCheckPosition(position);\n" +
            indent + "    }\n\n" +

            indent + "    SBE_NODISCARD inline std::uint64_t count() const SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return m_count;\n" +
            indent + "    }\n\n" +

            indent + "    SBE_NODISCARD inline bool hasNext() const SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return m_index < m_count;\n" +
            indent + "    }\n\n" +

            indent + "    inline %3$s &next()\n" +
            indent + "    {\n" +
            indent + "        if (m_index >= m_count)\n" +
            indent + "        {\n" +
            indent + "            throw std::runtime_error(\"index >= count [E108]\");\n" +
            indent + "        }\n" +
            "%4$s" +
            indent + "        m_offset = *m_positionPtr;\n" +
            indent + "        if (SBE_BOUNDS_CHECK_EXPECT(((m_offset + m_blockLength) > m_bufferLength), false))\n" +
            indent + "        {\n" +
            indent + "            throw std::runtime_error(\"buffer too short for next group index [E108]\");\n" +
            indent + "        }\n" +
            indent + "        *m_positionPtr = m_offset + m_blockLength;\n" +
            indent + "        ++m_index;\n\n" +

            indent + "        return *this;\n" +
            indent + "    }\n",
            dimensionHeaderLength,
            blockLength,
            groupClassName,
            onNextAccessOrderCall);

        sb.append("\n")
            .append(indent).append("    inline std::uint64_t resetCountToIndex()\n")
            .append(indent).append("    {\n")
            .append(generateAccessOrderListenerCall(fieldPrecedenceModel, indent + TWO_INDENT, "onResetCountToIndex"))
            .append(indent).append("        m_count = m_index;\n")
            .append(indent).append("        ").append(dimensionsClassName)
            .append(" dimensions(m_buffer, m_initialPosition, m_bufferLength, m_actingVersion);\n")
            .append(indent)
            .append("        dimensions.numInGroup(static_cast<").append(cppTypeNumInGroup).append(">(m_count));\n")
            .append(indent).append("        return m_count;\n")
            .append(indent).append("    }\n");

        sb.append("\n")
            .append(indent).append("    template<class Func> inline void forEach(Func &&func)\n")
            .append(indent).append("    {\n")
            .append(indent).append("        while (hasNext())\n")
            .append(indent).append("        {\n")
            .append(indent).append("            next();\n")
            .append(indent).append("            func(*this);\n")
            .append(indent).append("        }\n")
            .append(indent).append("    }\n\n");
    }

    private void generateGroupProperty(
        final StringBuilder sb,
        final String groupName,
        final Token token,
        final String cppTypeForNumInGroup,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent)
    {
        final String className = formatClassName(groupName);
        final String propertyName = formatPropertyName(groupName);

        new Formatter(sb).format("\n" +
            "private:\n" +
            indent + "    %1$s m_%2$s;\n\n" +

            "public:\n",
            className,
            propertyName);

        new Formatter(sb).format(
            indent + "    SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t %1$sId() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %2$d;\n" +
            indent + "    }\n",
            groupName,
            token.id());

        if (null != fieldPrecedenceModel)
        {
            generateAccessOrderListenerMethodForGroupWrap(
                sb,
                fieldPrecedenceModel,
                indent,
                token
            );
        }

        final String codecStateArgument = null == fieldPrecedenceModel ? "" : ", codecStatePtr()";

        final CharSequence onDecodeAccessOrderCall = null == fieldPrecedenceModel ? "" :
            generateAccessOrderListenerCall(fieldPrecedenceModel, indent + TWO_INDENT, token,
            "m_" + propertyName + ".count()", "\"decode\"");

        if (token.version() > 0)
        {
            new Formatter(sb).format("\n" +
                indent + "    SBE_NODISCARD inline %1$s &%2$s()\n" +
                indent + "    {\n" +
                indent + "        if (m_actingVersion < %5$du)\n" +
                indent + "        {\n" +
                indent + "            m_%2$s.notPresent(m_actingVersion);\n" +
                indent + "            return m_%2$s;\n" +
                indent + "        }\n\n" +

                indent + "        m_%2$s.wrapForDecode(" +
                "m_buffer, sbePositionPtr(), m_actingVersion, m_bufferLength%3$s);\n" +
                "%4$s" +
                indent + "        return m_%2$s;\n" +
                indent + "    }\n",
                className,
                propertyName,
                codecStateArgument,
                onDecodeAccessOrderCall,
                token.version());
        }
        else
        {
            new Formatter(sb).format("\n" +
                indent + "    SBE_NODISCARD inline %1$s &%2$s()\n" +
                indent + "    {\n" +
                indent + "        m_%2$s.wrapForDecode(" +
                "m_buffer, sbePositionPtr(), m_actingVersion, m_bufferLength%3$s);\n" +
                "%4$s" +
                indent + "        return m_%2$s;\n" +
                indent + "    }\n",
                className,
                propertyName,
                codecStateArgument,
                onDecodeAccessOrderCall);
        }

        final CharSequence onEncodeAccessOrderCall = null == fieldPrecedenceModel ? "" :
            generateAccessOrderListenerCall(fieldPrecedenceModel, indent + TWO_INDENT, token, "count", "\"encode\"");

        new Formatter(sb).format("\n" +
            indent + "    %1$s &%2$sCount(const %3$s count)\n" +
            indent + "    {\n" +
            indent + "        m_%2$s.wrapForEncode(" +
            "m_buffer, count, sbePositionPtr(), m_actingVersion, m_bufferLength%4$s);\n" +
            "%5$s" +
            indent + "        return m_%2$s;\n" +
            indent + "    }\n",
            className,
            propertyName,
            cppTypeForNumInGroup,
            codecStateArgument,
            onEncodeAccessOrderCall);

        final int version = token.version();
        final String versionCheck = 0 == version ?
            "        return true;\n" : "        return m_actingVersion >= %1$sSinceVersion();\n";
        new Formatter(sb).format("\n" +
            indent + "    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t %1$sSinceVersion() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %2$d;\n" +
            indent + "    }\n\n" +

            indent + "    SBE_NODISCARD bool %1$sInActingVersion() const SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + versionCheck +
            indent + "    }\n",
            propertyName,
            version);
    }

    private void generateVarData(
        final StringBuilder sb,
        final String className,
        final List<Token> tokens,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent)
    {
        for (int i = 0, size = tokens.size(); i < size;)
        {
            final Token token = tokens.get(i);
            if (token.signal() != Signal.BEGIN_VAR_DATA)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_VAR_DATA: token=" + token);
            }

            final String propertyName = toUpperFirstChar(token.name());
            final Token lengthToken = Generators.findFirst("length", tokens, i);
            final Token varDataToken = Generators.findFirst("varData", tokens, i);
            final String characterEncoding = varDataToken.encoding().characterEncoding();
            final int lengthOfLengthField = lengthToken.encodedLength();
            final String lengthCppType = cppTypeName(lengthToken.encoding().primitiveType());
            final String lengthByteOrderStr = formatByteOrderEncoding(
                lengthToken.encoding().byteOrder(), lengthToken.encoding().primitiveType());

            generateFieldMetaAttributeMethod(sb, token, indent);

            generateVarDataDescriptors(
                sb, token, propertyName, characterEncoding, lengthOfLengthField, indent);

            generateAccessOrderListenerMethodForVarDataLength(sb, fieldPrecedenceModel, indent, token);

            final CharSequence lengthAccessListenerCall = generateAccessOrderListenerCall(
                fieldPrecedenceModel, indent + TWO_INDENT,
                accessOrderListenerMethodName(token, "Length"));

            new Formatter(sb).format("\n" +
                indent + "    SBE_NODISCARD %4$s %1$sLength() const\n" +
                indent + "    {\n" +
                "%2$s" +
                "%5$s" +
                indent + "        %4$s length;\n" +
                indent + "        std::memcpy(&length, m_buffer + sbePosition(), sizeof(%4$s));\n" +
                indent + "        return %3$s(length);\n" +
                indent + "    }\n",
                toLowerFirstChar(propertyName),
                generateArrayFieldNotPresentCondition(token.version(), BASE_INDENT),
                formatByteOrderEncoding(lengthToken.encoding().byteOrder(), lengthToken.encoding().primitiveType()),
                lengthCppType,
                lengthAccessListenerCall);

            generateAccessOrderListenerMethod(sb, fieldPrecedenceModel, indent, token);

            final CharSequence accessOrderListenerCall =
                generateAccessOrderListenerCall(fieldPrecedenceModel, indent + TWO_INDENT, token);

            new Formatter(sb).format("\n" +
                indent + "    std::uint64_t skip%1$s()\n" +
                indent + "    {\n" +
                "%2$s" +
                "%6$s" +
                indent + "        std::uint64_t lengthOfLengthField = %3$d;\n" +
                indent + "        std::uint64_t lengthPosition = sbePosition();\n" +
                indent + "        %5$s lengthFieldValue;\n" +
                indent + "        std::memcpy(&lengthFieldValue, m_buffer + lengthPosition, sizeof(%5$s));\n" +
                indent + "        std::uint64_t dataLength = %4$s(lengthFieldValue);\n" +
                indent + "        sbePosition(lengthPosition + lengthOfLengthField + dataLength);\n" +
                indent + "        return dataLength;\n" +
                indent + "    }\n",
                propertyName,
                generateArrayFieldNotPresentCondition(token.version(), indent),
                lengthOfLengthField,
                lengthByteOrderStr,
                lengthCppType,
                accessOrderListenerCall);

            new Formatter(sb).format("\n" +
                indent + "    SBE_NODISCARD const char *%1$s()\n" +
                indent + "    {\n" +
                "%2$s" +
                "%6$s" +
                indent + "        %4$s lengthFieldValue;\n" +
                indent + "        std::memcpy(&lengthFieldValue, m_buffer + sbePosition(), sizeof(%4$s));\n" +
                indent + "        const char *fieldPtr = m_buffer + sbePosition() + %3$d;\n" +
                indent + "        sbePosition(sbePosition() + %3$d + %5$s(lengthFieldValue));\n" +
                indent + "        return fieldPtr;\n" +
                indent + "    }\n",
                formatPropertyName(propertyName),
                generateTypeFieldNotPresentCondition(token.version(), indent),
                lengthOfLengthField,
                lengthCppType,
                lengthByteOrderStr,
                accessOrderListenerCall);

            new Formatter(sb).format("\n" +
                indent + "    std::uint64_t get%1$s(char *dst, const std::uint64_t length)\n" +
                indent + "    {\n" +
                "%2$s" +
                "%6$s" +
                indent + "        std::uint64_t lengthOfLengthField = %3$d;\n" +
                indent + "        std::uint64_t lengthPosition = sbePosition();\n" +
                indent + "        sbePosition(lengthPosition + lengthOfLengthField);\n" +
                indent + "        %5$s lengthFieldValue;\n" +
                indent + "        std::memcpy(&lengthFieldValue, m_buffer + lengthPosition, sizeof(%5$s));\n" +
                indent + "        std::uint64_t dataLength = %4$s(lengthFieldValue);\n" +
                indent + "        std::uint64_t bytesToCopy = length < dataLength ? length : dataLength;\n" +
                indent + "        std::uint64_t pos = sbePosition();\n" +
                indent + "        sbePosition(pos + dataLength);\n" +
                indent + "        std::memcpy(dst, m_buffer + pos, static_cast<std::size_t>(bytesToCopy));\n" +
                indent + "        return bytesToCopy;\n" +
                indent + "    }\n",
                propertyName,
                generateArrayFieldNotPresentCondition(token.version(), indent),
                lengthOfLengthField,
                lengthByteOrderStr,
                lengthCppType,
                accessOrderListenerCall);

            new Formatter(sb).format("\n" +
                indent + "    %5$s &put%1$s(const char *src, const %3$s length)\n" +
                indent + "    {\n" +
                "%6$s" +
                indent + "        std::uint64_t lengthOfLengthField = %2$d;\n" +
                indent + "        std::uint64_t lengthPosition = sbePosition();\n" +
                indent + "        %3$s lengthFieldValue = %4$s(length);\n" +
                indent + "        sbePosition(lengthPosition + lengthOfLengthField);\n" +
                indent + "        std::memcpy(m_buffer + lengthPosition, &lengthFieldValue, sizeof(%3$s));\n" +
                indent + "        if (length != %3$s(0))\n" +
                indent + "        {\n" +
                indent + "            std::uint64_t pos = sbePosition();\n" +
                indent + "            sbePosition(pos + length);\n" +
                indent + "            std::memcpy(m_buffer + pos, src, length);\n" +
                indent + "        }\n" +
                indent + "        return *this;\n" +
                indent + "    }\n",
                propertyName,
                lengthOfLengthField,
                lengthCppType,
                lengthByteOrderStr,
                className,
                accessOrderListenerCall);

            new Formatter(sb).format("\n" +
                indent + "    std::string get%1$sAsString()\n" +
                indent + "    {\n" +
                "%2$s" +
                "%6$s" +
                indent + "        std::uint64_t lengthOfLengthField = %3$d;\n" +
                indent + "        std::uint64_t lengthPosition = sbePosition();\n" +
                indent + "        sbePosition(lengthPosition + lengthOfLengthField);\n" +
                indent + "        %5$s lengthFieldValue;\n" +
                indent + "        std::memcpy(&lengthFieldValue, m_buffer + lengthPosition, sizeof(%5$s));\n" +
                indent + "        std::uint64_t dataLength = %4$s(lengthFieldValue);\n" +
                indent + "        std::uint64_t pos = sbePosition();\n" +
                indent + "        const std::string result(m_buffer + pos, dataLength);\n" +
                indent + "        sbePosition(pos + dataLength);\n" +
                indent + "        return result;\n" +
                indent + "    }\n",
                propertyName,
                generateStringNotPresentCondition(token.version(), indent),
                lengthOfLengthField,
                lengthByteOrderStr,
                lengthCppType,
                accessOrderListenerCall);

            generateJsonEscapedStringGetter(sb, token, indent, propertyName, accessOrderListenerCall);

            new Formatter(sb).format("\n" +
                indent + "    #if __cplusplus >= 201703L\n" +
                indent + "    std::string_view get%1$sAsStringView()\n" +
                indent + "    {\n" +
                "%2$s" +
                "%6$s" +
                indent + "        std::uint64_t lengthOfLengthField = %3$d;\n" +
                indent + "        std::uint64_t lengthPosition = sbePosition();\n" +
                indent + "        sbePosition(lengthPosition + lengthOfLengthField);\n" +
                indent + "        %5$s lengthFieldValue;\n" +
                indent + "        std::memcpy(&lengthFieldValue, m_buffer + lengthPosition, sizeof(%5$s));\n" +
                indent + "        std::uint64_t dataLength = %4$s(lengthFieldValue);\n" +
                indent + "        std::uint64_t pos = sbePosition();\n" +
                indent + "        const std::string_view result(m_buffer + pos, dataLength);\n" +
                indent + "        sbePosition(pos + dataLength);\n" +
                indent + "        return result;\n" +
                indent + "    }\n" +
                indent + "    #endif\n",
                propertyName,
                generateStringViewNotPresentCondition(token.version(), indent),
                lengthOfLengthField,
                lengthByteOrderStr,
                lengthCppType,
                accessOrderListenerCall);

            new Formatter(sb).format("\n" +
                indent + "    %1$s &put%2$s(const std::string &str)\n" +
                indent + "    {\n" +
                indent + "        if (str.length() > %4$d)\n" +
                indent + "        {\n" +
                indent + "            throw std::runtime_error(\"std::string too long for length type [E109]\");\n" +
                indent + "        }\n" +
                indent + "        return put%2$s(str.data(), static_cast<%3$s>(str.length()));\n" +
                indent + "    }\n",
                className,
                propertyName,
                lengthCppType,
                lengthToken.encoding().applicableMaxValue().longValue());

            new Formatter(sb).format("\n" +
                indent + "    #if __cplusplus >= 201703L\n" +
                indent + "    %1$s &put%2$s(const std::string_view str)\n" +
                indent + "    {\n" +
                indent + "        if (str.length() > %4$d)\n" +
                indent + "        {\n" +
                indent + "            throw std::runtime_error(\"std::string too long for length type [E109]\");\n" +
                indent + "        }\n" +
                indent + "        return put%2$s(str.data(), static_cast<%3$s>(str.length()));\n" +
                indent + "    }\n" +
                indent + "    #endif\n",
                className,
                propertyName,
                lengthCppType,
                lengthToken.encoding().applicableMaxValue().longValue());

            i += token.componentTokenCount();
        }
    }

    private void generateVarDataDescriptors(
        final StringBuilder sb,
        final Token token,
        final String propertyName,
        final String characterEncoding,
        final Integer sizeOfLengthField,
        final String indent)
    {
        new Formatter(sb).format("\n" +
            indent + "    static const char *%1$sCharacterEncoding() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return \"%2$s\";\n" +
            indent + "    }\n",
            toLowerFirstChar(propertyName),
            characterEncoding);

        final int version = token.version();
        final String versionCheck = 0 == version ?
            "        return true;\n" : "        return m_actingVersion >= %1$sSinceVersion();\n";
        new Formatter(sb).format("\n" +
            indent + "    static SBE_CONSTEXPR std::uint64_t %1$sSinceVersion() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %2$d;\n" +
            indent + "    }\n\n" +

            indent + "    bool %1$sInActingVersion() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + versionCheck +
            indent + "    }\n\n" +

            indent + "    static SBE_CONSTEXPR std::uint16_t %1$sId() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %3$d;\n" +
            indent + "    }\n",
            toLowerFirstChar(propertyName),
            version,
            token.id());

        new Formatter(sb).format("\n" +
            indent + "    static SBE_CONSTEXPR std::uint64_t %sHeaderLength() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %d;\n" +
            indent + "    }\n",
            toLowerFirstChar(propertyName),
            sizeOfLengthField);
    }

    private void generateChoiceSet(final List<Token> tokens) throws IOException
    {
        final Token token = tokens.get(0);
        final String bitSetName = formatClassName(token.applicableTypeName());

        try (Writer out = outputManager.createOutput(bitSetName))
        {
            out.append(generateFileHeader(ir.namespaces(), bitSetName, null));
            out.append(generateClassDeclaration(bitSetName));
            out.append(generateFixedFlyweightCode(bitSetName, token.encodedLength()));

            final Encoding encoding = token.encoding();
            new Formatter(out).format("\n" +
                "    %1$s &clear()\n" +
                "    {\n" +
                "        %2$s zero = 0;\n" +
                "        std::memcpy(m_buffer + m_offset, &zero, sizeof(%2$s));\n" +
                "        return *this;\n" +
                "    }\n",
                bitSetName,
                cppTypeName(encoding.primitiveType()));

            new Formatter(out).format("\n" +
                "    SBE_NODISCARD bool isEmpty() const\n" +
                "    {\n" +
                "        %1$s val;\n" +
                "        std::memcpy(&val, m_buffer + m_offset, sizeof(%1$s));\n" +
                "        return 0 == val;\n" +
                "    }\n",
                cppTypeName(encoding.primitiveType()));

            new Formatter(out).format("\n" +
                "    SBE_NODISCARD %1$s rawValue() const\n" +
                "    {\n" +
                "        %1$s val;\n" +
                "        std::memcpy(&val, m_buffer + m_offset, sizeof(%1$s));\n" +
                "        return val;\n" +
                "    }\n",
                cppTypeName(encoding.primitiveType()));

            new Formatter(out).format("\n" +
                "    %1$s &rawValue(%2$s value)\n" +
                "    {\n" +
                "        std::memcpy(m_buffer + m_offset, &value, sizeof(%2$s));\n" +
                "        return *this;\n" +
                "    }\n",
                bitSetName,
                cppTypeName(encoding.primitiveType()));

            out.append(generateChoices(bitSetName, tokens.subList(1, tokens.size() - 1)));
            out.append(generateChoicesDisplay(bitSetName, tokens.subList(1, tokens.size() - 1)));
            out.append("};\n");
            out.append(CppUtil.closingBraces(ir.namespaces().length)).append("#endif\n");
        }
    }

    private void generateEnum(final List<Token> tokens) throws IOException
    {
        final Token enumToken = tokens.get(0);
        final String enumName = formatClassName(tokens.get(0).applicableTypeName());

        try (Writer out = outputManager.createOutput(enumName))
        {
            out.append(generateEnumFileHeader(ir.namespaces(), enumName));
            out.append(generateEnumDeclaration(enumName));

            out.append(generateEnumValues(tokens.subList(1, tokens.size() - 1), enumToken));

            out.append(generateEnumLookupMethod(tokens.subList(1, tokens.size() - 1), enumToken));

            out.append(generateEnumDisplay(tokens.subList(1, tokens.size() - 1), enumToken));

            out.append("};\n\n");
            out.append(CppUtil.closingBraces(ir.namespaces().length)).append("\n#endif\n");
        }
    }

    private void generateComposite(final List<Token> tokens) throws IOException
    {
        final String compositeName = formatClassName(tokens.get(0).applicableTypeName());

        try (Writer out = outputManager.createOutput(compositeName))
        {
            out.append(generateFileHeader(ir.namespaces(), compositeName,
                generateTypesToIncludes(tokens.subList(1, tokens.size() - 1))));
            out.append(generateClassDeclaration(compositeName));
            out.append(generateFixedFlyweightCode(compositeName, tokens.get(0).encodedLength()));

            out.append(generateCompositePropertyElements(
                compositeName, tokens.subList(1, tokens.size() - 1), BASE_INDENT));

            out.append(generateCompositeDisplay(
                tokens.get(0).applicableTypeName(), tokens.subList(1, tokens.size() - 1)));

            out.append("};\n\n");
            out.append(CppUtil.closingBraces(ir.namespaces().length)).append("\n#endif\n");
        }
    }

    private static CharSequence generateChoiceNotPresentCondition(final int sinceVersion)
    {
        if (0 == sinceVersion)
        {
            return "";
        }

        return String.format(
            "        if (m_actingVersion < %1$d)\n" +
            "        {\n" +
            "            return false;\n" +
            "        }\n\n",
            sinceVersion);
    }

    private CharSequence generateChoices(final String bitsetClassName, final List<Token> tokens)
    {
        final StringBuilder sb = new StringBuilder();

        tokens
            .stream()
            .filter((token) -> token.signal() == Signal.CHOICE)
            .forEach((token) ->
            {
                final String choiceName = formatPropertyName(token.name());
                final PrimitiveType type = token.encoding().primitiveType();
                final String typeName = cppTypeName(type);
                final String choiceBitPosition = token.encoding().constValue().toString();
                final String byteOrderStr = formatByteOrderEncoding(token.encoding().byteOrder(), type);
                final CharSequence constantOne = generateLiteral(type, "1");

                new Formatter(sb).format("\n" +
                    "    static bool %1$s(const %2$s bits)\n" +
                    "    {\n" +
                    "        return (bits & (%4$s << %3$su)) != 0;\n" +
                    "    }\n",
                    choiceName,
                    typeName,
                    choiceBitPosition,
                    constantOne);

                new Formatter(sb).format("\n" +
                    "    static %2$s %1$s(const %2$s bits, const bool value)\n" +
                    "    {\n" +
                    "        return value ?" +
                    " static_cast<%2$s>(bits | (%4$s << %3$su)) : static_cast<%2$s>(bits & ~(%4$s << %3$su));\n" +
                    "    }\n",
                    choiceName,
                    typeName,
                    choiceBitPosition,
                    constantOne);

                new Formatter(sb).format("\n" +
                    "    SBE_NODISCARD bool %1$s() const\n" +
                    "    {\n" +
                    "%2$s" +
                    "        %4$s val;\n" +
                    "        std::memcpy(&val, m_buffer + m_offset, sizeof(%4$s));\n" +
                    "        return (%3$s(val) & (%6$s << %5$su)) != 0;\n" +
                    "    }\n",
                    choiceName,
                    generateChoiceNotPresentCondition(token.version()),
                    byteOrderStr,
                    typeName,
                    choiceBitPosition,
                    constantOne);

                new Formatter(sb).format("\n" +
                    "    %1$s &%2$s(const bool value)\n" +
                    "    {\n" +
                    "        %3$s bits;\n" +
                    "        std::memcpy(&bits, m_buffer + m_offset, sizeof(%3$s));\n" +
                    "        bits = %4$s(value ?" +
                    " static_cast<%3$s>(%4$s(bits) | (%6$s << %5$su)) " +
                    ": static_cast<%3$s>(%4$s(bits) & ~(%6$s << %5$su)));\n" +
                    "        std::memcpy(m_buffer + m_offset, &bits, sizeof(%3$s));\n" +
                    "        return *this;\n" +
                    "    }\n",
                    bitsetClassName,
                    choiceName,
                    typeName,
                    byteOrderStr,
                    choiceBitPosition,
                    constantOne);
            });

        return sb;
    }

    private CharSequence generateEnumValues(final List<Token> tokens, final Token encodingToken)
    {
        final StringBuilder sb = new StringBuilder();
        final Encoding encoding = encodingToken.encoding();

        sb.append(
            "    enum Value\n" +
            "    {\n");

        for (final Token token : tokens)
        {
            final CharSequence constVal = generateLiteral(
                token.encoding().primitiveType(), token.encoding().constValue().toString());
            sb.append("        ").append(token.name()).append(" = ").append(constVal).append(",\n");
        }

        final CharSequence nullLiteral = generateLiteral(
            encoding.primitiveType(), encoding.applicableNullValue().toString());
        if (shouldDecodeUnknownEnumValues)
        {
            sb.append("        SBE_UNKNOWN = ").append(nullLiteral).append(",\n");
        }

        sb.append("        NULL_VALUE = ").append(nullLiteral);
        sb.append("\n    };\n\n");

        return sb;
    }

    private CharSequence generateEnumLookupMethod(final List<Token> tokens, final Token encodingToken)
    {
        final String enumName = formatClassName(encodingToken.applicableTypeName());
        final StringBuilder sb = new StringBuilder();

        new Formatter(sb).format(
            "    static %1$s::Value get(const %2$s value)\n" +
            "    {\n" +
            "        switch (value)\n" +
            "        {\n",
            enumName,
            cppTypeName(tokens.get(0).encoding().primitiveType()));

        for (final Token token : tokens)
        {
            final CharSequence constVal = generateLiteral(
                token.encoding().primitiveType(), token.encoding().constValue().toString());

            sb.append("            case ").append(constVal).append(": return ").append(token.name()).append(";\n");
        }

        final CharSequence nullVal = generateLiteral(
            encodingToken.encoding().primitiveType(), encodingToken.encoding().applicableNullValue().toString());

        sb.append("            case ").append(nullVal).append(": return NULL_VALUE;\n")
            .append("        }\n\n");

        if (shouldDecodeUnknownEnumValues)
        {
            sb.append("        return SBE_UNKNOWN;\n").append("    }\n");
        }
        else
        {
            new Formatter(sb).format(
                "        throw std::runtime_error(\"unknown value for enum %s [E103]\");\n" +
                "    }\n",
                enumName);
        }

        return sb;
    }

    private CharSequence generateFieldNotPresentCondition(
        final int sinceVersion, final Encoding encoding, final String indent)
    {
        if (0 == sinceVersion)
        {
            return "";
        }

        return String.format(
            indent + "        if (m_actingVersion < %1$d)\n" +
            indent + "        {\n" +
            indent + "            return %2$s;\n" +
            indent + "        }\n\n",
            sinceVersion,
            generateLiteral(encoding.primitiveType(), encoding.applicableNullValue().toString()));
    }

    private static CharSequence generateArrayFieldNotPresentCondition(final int sinceVersion, final String indent)
    {
        if (0 == sinceVersion)
        {
            return "";
        }

        return String.format(
            indent + "        if (m_actingVersion < %1$d)\n" +
            indent + "        {\n" +
            indent + "            return 0;\n" +
            indent + "        }\n\n",
            sinceVersion);
    }

    private static CharSequence generateStringNotPresentCondition(final int sinceVersion, final String indent)
    {
        if (0 == sinceVersion)
        {
            return "";
        }

        return String.format(
            indent + "        if (m_actingVersion < %1$d)\n" +
            indent + "        {\n" +
            indent + "            return std::string(\"\");\n" +
            indent + "        }\n\n",
            sinceVersion);
    }

    private static CharSequence generateStringViewNotPresentCondition(final int sinceVersion, final String indent)
    {
        if (0 == sinceVersion)
        {
            return "";
        }

        return String.format(
            indent + "        if (m_actingVersion < %1$d)\n" +
            indent + "        {\n" +
            indent + "            return std::string_view(\"\");\n" +
            indent + "        }\n\n",
            sinceVersion);
    }

    private static CharSequence generateTypeFieldNotPresentCondition(final int sinceVersion, final String indent)
    {
        if (0 == sinceVersion)
        {
            return "";
        }

        return String.format(
            indent + "        if (m_actingVersion < %1$d)\n" +
            indent + "        {\n" +
            indent + "            return nullptr;\n" +
            indent + "        }\n\n",
            sinceVersion);
    }

    private static CharSequence generateFileHeader(
        final CharSequence[] namespaces,
        final String className,
        final List<String> typesToInclude)
    {
        final StringBuilder sb = new StringBuilder();

        sb.append("/* Generated @" + "generated SBE (Simple Binary Encoding) message codec */\n");

        sb.append(String.format(
            "#ifndef _%1$s_%2$s_CXX_H_\n" +
            "#define _%1$s_%2$s_CXX_H_\n\n" +

            "#if __cplusplus >= 201103L\n" +
            "#  define SBE_CONSTEXPR constexpr\n" +
            "#  define SBE_NOEXCEPT noexcept\n" +
            "#else\n" +
            "#  define SBE_CONSTEXPR\n" +
            "#  define SBE_NOEXCEPT\n" +
            "#endif\n\n" +

            "#if __cplusplus >= 201703L\n" +
            "#  include <string_view>\n" +
            "#  define SBE_NODISCARD [[nodiscard]]\n" +
            "#else\n" +
            "#  define SBE_NODISCARD\n" +
            "#endif\n\n" +

            "#if !defined(__STDC_LIMIT_MACROS)\n" +
            "#  define __STDC_LIMIT_MACROS 1\n" +
            "#endif\n\n" +

            "#include <cstdint>\n" +
            "#include <limits>\n" +
            "#include <cstring>\n" +
            "#include <iomanip>\n" +
            "#include <ostream>\n" +
            "#include <stdexcept>\n" +
            "#include <sstream>\n" +
            "#include <string>\n" +
            "#include <vector>\n" +
            "#include <tuple>\n" +
            "\n" +

            "#if defined(WIN32) || defined(_WIN32)\n" +
            "#  define SBE_BIG_ENDIAN_ENCODE_16(v) _byteswap_ushort(v)\n" +
            "#  define SBE_BIG_ENDIAN_ENCODE_32(v) _byteswap_ulong(v)\n" +
            "#  define SBE_BIG_ENDIAN_ENCODE_64(v) _byteswap_uint64(v)\n" +
            "#  define SBE_LITTLE_ENDIAN_ENCODE_16(v) (v)\n" +
            "#  define SBE_LITTLE_ENDIAN_ENCODE_32(v) (v)\n" +
            "#  define SBE_LITTLE_ENDIAN_ENCODE_64(v) (v)\n" +
            "#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__\n" +
            "#  define SBE_BIG_ENDIAN_ENCODE_16(v) __builtin_bswap16(v)\n" +
            "#  define SBE_BIG_ENDIAN_ENCODE_32(v) __builtin_bswap32(v)\n" +
            "#  define SBE_BIG_ENDIAN_ENCODE_64(v) __builtin_bswap64(v)\n" +
            "#  define SBE_LITTLE_ENDIAN_ENCODE_16(v) (v)\n" +
            "#  define SBE_LITTLE_ENDIAN_ENCODE_32(v) (v)\n" +
            "#  define SBE_LITTLE_ENDIAN_ENCODE_64(v) (v)\n" +
            "#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__\n" +
            "#  define SBE_LITTLE_ENDIAN_ENCODE_16(v) __builtin_bswap16(v)\n" +
            "#  define SBE_LITTLE_ENDIAN_ENCODE_32(v) __builtin_bswap32(v)\n" +
            "#  define SBE_LITTLE_ENDIAN_ENCODE_64(v) __builtin_bswap64(v)\n" +
            "#  define SBE_BIG_ENDIAN_ENCODE_16(v) (v)\n" +
            "#  define SBE_BIG_ENDIAN_ENCODE_32(v) (v)\n" +
            "#  define SBE_BIG_ENDIAN_ENCODE_64(v) (v)\n" +
            "#else\n" +
            "#  error \"Byte Ordering of platform not determined. " +
            "Set __BYTE_ORDER__ manually before including this file.\"\n" +
            "#endif\n\n" +

            "#if !defined(SBE_BOUNDS_CHECK_EXPECT)\n" +
            "#  if defined(SBE_NO_BOUNDS_CHECK)\n" +
            "#    define SBE_BOUNDS_CHECK_EXPECT(exp, c) (false)\n" +
            "#  elif defined(_MSC_VER)\n" +
            "#    define SBE_BOUNDS_CHECK_EXPECT(exp, c) (exp)\n" +
            "#  else \n" +
            "#    define SBE_BOUNDS_CHECK_EXPECT(exp, c) (__builtin_expect(exp, c))\n" +
            "#  endif\n\n" +
            "#endif\n\n" +

            "#define SBE_FLOAT_NAN std::numeric_limits<float>::quiet_NaN()\n" +
            "#define SBE_DOUBLE_NAN std::numeric_limits<double>::quiet_NaN()\n" +
            "#define SBE_NULLVALUE_INT8 (std::numeric_limits<std::int8_t>::min)()\n" +
            "#define SBE_NULLVALUE_INT16 (std::numeric_limits<std::int16_t>::min)()\n" +
            "#define SBE_NULLVALUE_INT32 (std::numeric_limits<std::int32_t>::min)()\n" +
            "#define SBE_NULLVALUE_INT64 (std::numeric_limits<std::int64_t>::min)()\n" +
            "#define SBE_NULLVALUE_UINT8 (std::numeric_limits<std::uint8_t>::max)()\n" +
            "#define SBE_NULLVALUE_UINT16 (std::numeric_limits<std::uint16_t>::max)()\n" +
            "#define SBE_NULLVALUE_UINT32 (std::numeric_limits<std::uint32_t>::max)()\n" +
            "#define SBE_NULLVALUE_UINT64 (std::numeric_limits<std::uint64_t>::max)()\n\n",
            String.join("_", namespaces).toUpperCase(),
            className.toUpperCase()));

        if (typesToInclude != null && !typesToInclude.isEmpty())
        {
            sb.append("\n");
            for (final String incName : typesToInclude)
            {
                sb.append("#include \"").append(toUpperFirstChar(incName)).append(".h\"\n");
            }
        }

        sb.append("\nnamespace ");
        sb.append(String.join(" {\nnamespace ", namespaces));
        sb.append(" {\n\n");

        return sb;
    }

    private static CharSequence generateEnumFileHeader(final CharSequence[] namespaces, final String className)
    {
        final StringBuilder sb = new StringBuilder();

        sb.append("/* Generated @" + "generated SBE (Simple Binary Encoding) message codec */\n");

        sb.append(String.format(
            "#ifndef _%1$s_%2$s_CXX_H_\n" +
            "#define _%1$s_%2$s_CXX_H_\n\n" +

            "#if !defined(__STDC_LIMIT_MACROS)\n" +
            "#  define __STDC_LIMIT_MACROS 1\n" +
            "#endif\n\n" +

            "#include <cstdint>\n" +
            "#include <iomanip>\n" +
            "#include <limits>\n" +
            "#include <ostream>\n" +
            "#include <stdexcept>\n" +
            "#include <sstream>\n" +
            "#include <string>\n" +
            "\n" +

            "#define SBE_NULLVALUE_INT8 (std::numeric_limits<std::int8_t>::min)()\n" +
            "#define SBE_NULLVALUE_INT16 (std::numeric_limits<std::int16_t>::min)()\n" +
            "#define SBE_NULLVALUE_INT32 (std::numeric_limits<std::int32_t>::min)()\n" +
            "#define SBE_NULLVALUE_INT64 (std::numeric_limits<std::int64_t>::min)()\n" +
            "#define SBE_NULLVALUE_UINT8 (std::numeric_limits<std::uint8_t>::max)()\n" +
            "#define SBE_NULLVALUE_UINT16 (std::numeric_limits<std::uint16_t>::max)()\n" +
            "#define SBE_NULLVALUE_UINT32 (std::numeric_limits<std::uint32_t>::max)()\n" +
            "#define SBE_NULLVALUE_UINT64 (std::numeric_limits<std::uint64_t>::max)()\n",
            String.join("_", namespaces).toUpperCase(),
            className.toUpperCase()));

        sb.append("\nnamespace ");
        sb.append(String.join(" {\nnamespace ", namespaces));
        sb.append(" {\n\n");

        return sb;
    }

    private static CharSequence generateClassDeclaration(final String className)
    {
        return
            "class " + className + "\n" +
            "{\n";
    }

    private static CharSequence generateEnumDeclaration(final String name)
    {
        return "class " + name + "\n{\npublic:\n";
    }

    private CharSequence generateCompositePropertyElements(
        final String containingClassName, final List<Token> tokens, final String indent)
    {
        final StringBuilder sb = new StringBuilder();

        for (int i = 0; i < tokens.size();)
        {
            final Token fieldToken = tokens.get(i);
            final String propertyName = formatPropertyName(fieldToken.name());

            generateFieldMetaAttributeMethod(sb, fieldToken, indent);
            generateFieldCommonMethods(indent, sb, fieldToken, fieldToken, propertyName);

            switch (fieldToken.signal())
            {
                case ENCODING:
                    generatePrimitiveProperty(sb, containingClassName, propertyName, fieldToken, fieldToken,
                        null, indent);
                    break;

                case BEGIN_ENUM:
                    generateEnumProperty(sb, containingClassName, fieldToken, propertyName, fieldToken,
                        null, indent);
                    break;

                case BEGIN_SET:
                    generateBitsetProperty(sb, propertyName, fieldToken, fieldToken, null, indent);
                    break;

                case BEGIN_COMPOSITE:
                    generateCompositeProperty(sb, propertyName, fieldToken, fieldToken, null, indent);
                    break;

                default:
                    break;
            }

            i += tokens.get(i).componentTokenCount();
        }

        return sb;
    }

    private void generatePrimitiveProperty(
        final StringBuilder sb,
        final String containingClassName,
        final String propertyName,
        final Token propertyToken,
        final Token encodingToken,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent)
    {
        generatePrimitiveFieldMetaData(sb, propertyName, encodingToken, indent);

        if (encodingToken.isConstantEncoding())
        {
            generateConstPropertyMethods(sb, propertyName, encodingToken, indent);
        }
        else
        {
            generatePrimitivePropertyMethods(
                sb, containingClassName, propertyName, propertyToken, encodingToken, fieldPrecedenceModel, indent);
        }
    }

    private void generatePrimitivePropertyMethods(
        final StringBuilder sb,
        final String containingClassName,
        final String propertyName,
        final Token propertyToken,
        final Token encodingToken,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent)
    {
        final int arrayLength = encodingToken.arrayLength();
        if (arrayLength == 1)
        {
            generateSingleValueProperty(sb, containingClassName, propertyName, propertyToken, encodingToken,
                fieldPrecedenceModel, indent);
        }
        else if (arrayLength > 1)
        {
            generateArrayProperty(sb, containingClassName, propertyName, propertyToken, encodingToken,
                fieldPrecedenceModel, indent);
        }
    }

    private void generatePrimitiveFieldMetaData(
        final StringBuilder sb, final String propertyName, final Token token, final String indent)
    {
        final Encoding encoding = token.encoding();
        final PrimitiveType primitiveType = encoding.primitiveType();
        final String cppTypeName = cppTypeName(primitiveType);
        final CharSequence nullValueString = generateNullValueLiteral(primitiveType, encoding);

        new Formatter(sb).format("\n" +
            indent + "    static SBE_CONSTEXPR %1$s %2$sNullValue() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %3$s;\n" +
            indent + "    }\n",
            cppTypeName,
            propertyName,
            nullValueString);

        new Formatter(sb).format("\n" +
            indent + "    static SBE_CONSTEXPR %1$s %2$sMinValue() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %3$s;\n" +
            indent + "    }\n",
            cppTypeName,
            propertyName,
            generateLiteral(primitiveType, token.encoding().applicableMinValue().toString()));

        new Formatter(sb).format("\n" +
            indent + "    static SBE_CONSTEXPR %1$s %2$sMaxValue() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %3$s;\n" +
            indent + "    }\n",
            cppTypeName,
            propertyName,
            generateLiteral(primitiveType, token.encoding().applicableMaxValue().toString()));

        new Formatter(sb).format("\n" +
            indent + "    static SBE_CONSTEXPR std::size_t %1$sEncodingLength() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %2$d;\n" +
            indent + "    }\n",
            propertyName,
            token.encoding().primitiveType().size() * token.arrayLength());
    }

    private CharSequence generateLoadValue(
        final PrimitiveType primitiveType,
        final String offsetStr,
        final ByteOrder byteOrder,
        final String indent)
    {
        final String cppTypeName = cppTypeName(primitiveType);
        final String byteOrderStr = formatByteOrderEncoding(byteOrder, primitiveType);
        final StringBuilder sb = new StringBuilder();

        if (primitiveType == PrimitiveType.FLOAT || primitiveType == PrimitiveType.DOUBLE)
        {
            final String stackUnion =
                primitiveType == PrimitiveType.FLOAT ? "union sbe_float_as_uint_u" : "union sbe_double_as_uint_u";

            new Formatter(sb).format(
                indent + "        %1$s val;\n" +
                indent + "        std::memcpy(&val, m_buffer + m_offset + %2$s, sizeof(%3$s));\n" +
                indent + "        val.uint_value = %4$s(val.uint_value);\n" +
                indent + "        return val.fp_value;\n",
                stackUnion,
                offsetStr,
                cppTypeName,
                byteOrderStr);
        }
        else
        {
            new Formatter(sb).format(
                indent + "        %1$s val;\n" +
                indent + "        std::memcpy(&val, m_buffer + m_offset + %2$s, sizeof(%1$s));\n" +
                indent + "        return %3$s(val);\n",
                cppTypeName,
                offsetStr,
                byteOrderStr);
        }

        return sb;
    }

    private CharSequence generateStoreValue(
        final PrimitiveType primitiveType,
        final String valueSuffix,
        final String offsetStr,
        final ByteOrder byteOrder,
        final String indent)
    {
        final String cppTypeName = cppTypeName(primitiveType);
        final String byteOrderStr = formatByteOrderEncoding(byteOrder, primitiveType);
        final StringBuilder sb = new StringBuilder();

        if (primitiveType == PrimitiveType.FLOAT || primitiveType == PrimitiveType.DOUBLE)
        {
            final String stackUnion = primitiveType == PrimitiveType.FLOAT ?
                "union sbe_float_as_uint_u" : "union sbe_double_as_uint_u";

            new Formatter(sb).format(
                indent + "        %1$s val%2$s;\n" +
                indent + "        val%2$s.fp_value = value%2$s;\n" +
                indent + "        val%2$s.uint_value = %3$s(val%2$s.uint_value);\n" +
                indent + "        std::memcpy(m_buffer + m_offset + %4$s, &val%2$s, sizeof(%5$s));\n",
                stackUnion,
                valueSuffix,
                byteOrderStr,
                offsetStr,
                cppTypeName);
        }
        else
        {
            new Formatter(sb).format(
                indent + "        %1$s val%2$s = %3$s(value%2$s);\n" +
                indent + "        std::memcpy(m_buffer + m_offset + %4$s, &val%2$s, sizeof(%1$s));\n",
                cppTypeName,
                valueSuffix,
                byteOrderStr,
                offsetStr);
        }

        return sb;
    }

    private static String noexceptDeclaration(final FieldPrecedenceModel fieldPrecedenceModel)
    {
        return fieldPrecedenceModel == null ? " SBE_NOEXCEPT" : "";
    }

    private void generateSingleValueProperty(
        final StringBuilder sb,
        final String containingClassName,
        final String propertyName,
        final Token propertyToken,
        final Token encodingToken,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent)
    {
        final PrimitiveType primitiveType = encodingToken.encoding().primitiveType();
        final String cppTypeName = cppTypeName(primitiveType);
        final int offset = encodingToken.offset();

        final CharSequence accessOrderListenerCall =
            generateAccessOrderListenerCall(fieldPrecedenceModel, indent + TWO_INDENT, propertyToken);

        final String noexceptDeclaration = noexceptDeclaration(fieldPrecedenceModel);

        new Formatter(sb).format("\n" +
            indent + "    SBE_NODISCARD %1$s %2$s() const%6$s\n" +
            indent + "    {\n" +
            "%3$s" +
            "%4$s" +
            "%5$s" +
            indent + "    }\n",
            cppTypeName,
            propertyName,
            generateFieldNotPresentCondition(propertyToken.version(), encodingToken.encoding(), indent),
            accessOrderListenerCall,
            generateLoadValue(primitiveType, Integer.toString(offset), encodingToken.encoding().byteOrder(), indent),
            noexceptDeclaration);

        final CharSequence storeValue = generateStoreValue(
            primitiveType, "", Integer.toString(offset), encodingToken.encoding().byteOrder(), indent);

        new Formatter(sb).format("\n" +
            indent + "    %1$s &%2$s(const %3$s value)%6$s\n" +
            indent + "    {\n" +
            "%4$s" +
            "%5$s" +
            indent + "        return *this;\n" +
            indent + "    }\n",
            formatClassName(containingClassName),
            propertyName,
            cppTypeName,
            storeValue,
            accessOrderListenerCall,
            noexceptDeclaration);
    }

    private void generateArrayProperty(
        final StringBuilder sb,
        final String containingClassName,
        final String propertyName,
        final Token propertyToken,
        final Token encodingToken,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent)
    {
        final PrimitiveType primitiveType = encodingToken.encoding().primitiveType();
        final String cppTypeName = cppTypeName(primitiveType);
        final int offset = encodingToken.offset();

        final CharSequence accessOrderListenerCall =
            generateAccessOrderListenerCall(fieldPrecedenceModel, indent + TWO_INDENT, propertyToken);

        final String noexceptDeclaration = noexceptDeclaration(fieldPrecedenceModel);

        final int arrayLength = encodingToken.arrayLength();
        new Formatter(sb).format("\n" +
            indent + "    static SBE_CONSTEXPR std::uint64_t %1$sLength() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %2$d;\n" +
            indent + "    }\n",
            propertyName,
            arrayLength);

        new Formatter(sb).format("\n" +
            indent + "    SBE_NODISCARD const char *%1$s() const%5$s\n" +
            indent + "    {\n" +
            "%2$s" +
            "%4$s" +
            indent + "        return m_buffer + m_offset + %3$d;\n" +
            indent + "    }\n",
            propertyName,
            generateTypeFieldNotPresentCondition(propertyToken.version(), indent),
            offset,
            accessOrderListenerCall,
            noexceptDeclaration);

        new Formatter(sb).format("\n" +
            indent + "    SBE_NODISCARD char *%1$s()%5$s\n" +
            indent + "    {\n" +
            "%2$s" +
            "%4$s" +
            indent + "        return m_buffer + m_offset + %3$d;\n" +
            indent + "    }\n",
            propertyName,
            generateTypeFieldNotPresentCondition(propertyToken.version(), indent),
            offset,
            accessOrderListenerCall,
            noexceptDeclaration);

        final CharSequence loadValue = generateLoadValue(
            primitiveType,
            String.format("%d + (index * %d)", offset, primitiveType.size()),
            encodingToken.encoding().byteOrder(),
            indent);

        new Formatter(sb).format("\n" +
            indent + "    SBE_NODISCARD %1$s %2$s(const std::uint64_t index) const\n" +
            indent + "    {\n" +
            indent + "        if (index >= %3$d)\n" +
            indent + "        {\n" +
            indent + "            throw std::runtime_error(\"index out of range for %2$s [E104]\");\n" +
            indent + "        }\n\n" +
            "%4$s" +
            "%6$s" +
            "%5$s" +
            indent + "    }\n",
            cppTypeName,
            propertyName,
            arrayLength,
            generateFieldNotPresentCondition(propertyToken.version(), encodingToken.encoding(), indent),
            loadValue,
            accessOrderListenerCall);

        final CharSequence storeValue = generateStoreValue(
            primitiveType,
            "",
            String.format("%d + (index * %d)", offset, primitiveType.size()),
            encodingToken.encoding().byteOrder(),
            indent);

        new Formatter(sb).format("\n" +
            indent + "    %1$s &%2$s(const std::uint64_t index, const %3$s value)\n" +
            indent + "    {\n" +
            indent + "        if (index >= %4$d)\n" +
            indent + "        {\n" +
            indent + "            throw std::runtime_error(\"index out of range for %2$s [E105]\");\n" +
            indent + "        }\n\n" +

            "%6$s" +
            "%5$s" +
            indent + "        return *this;\n" +
            indent + "    }\n",
            containingClassName,
            propertyName,
            cppTypeName,
            arrayLength,
            storeValue,
            accessOrderListenerCall);

        new Formatter(sb).format("\n" +
            indent + "    std::uint64_t get%1$s(char *const dst, const std::uint64_t length) const\n" +
            indent + "    {\n" +
            indent + "        if (length > %2$d)\n" +
            indent + "        {\n" +
            indent + "            throw std::runtime_error(\"length too large for get%1$s [E106]\");\n" +
            indent + "        }\n\n" +

            "%3$s" +
            "%6$s" +
            indent + "        std::memcpy(dst, m_buffer + m_offset + %4$d, " +
            "sizeof(%5$s) * static_cast<std::size_t>(length));\n" +
            indent + "        return length;\n" +
            indent + "    }\n",
            toUpperFirstChar(propertyName),
            arrayLength,
            generateArrayFieldNotPresentCondition(propertyToken.version(), indent),
            offset,
            cppTypeName,
            accessOrderListenerCall);

        new Formatter(sb).format("\n" +
            indent + "    %1$s &put%2$s(const char *const src)%7$s\n" +
            indent + "    {\n" +
            "%6$s" +
            indent + "        std::memcpy(m_buffer + m_offset + %3$d, src, sizeof(%4$s) * %5$d);\n" +
            indent + "        return *this;\n" +
            indent + "    }\n",
            containingClassName,
            toUpperFirstChar(propertyName),
            offset,
            cppTypeName,
            arrayLength,
            accessOrderListenerCall,
            noexceptDeclaration);

        if (arrayLength > 1 && arrayLength <= 4)
        {
            sb.append("\n").append(indent).append("    ")
                .append(containingClassName).append(" &put").append(toUpperFirstChar(propertyName))
                .append("(\n");

            for (int i = 0; i < arrayLength; i++)
            {
                sb.append(indent).append("        ")
                    .append("const ").append(cppTypeName).append(" value").append(i);

                if (i < (arrayLength - 1))
                {
                    sb.append(",\n");
                }
            }

            sb.append(")").append(noexceptDeclaration).append("\n");
            sb.append(indent).append("    {\n");
            sb.append(accessOrderListenerCall);

            for (int i = 0; i < arrayLength; i++)
            {
                sb.append(generateStoreValue(
                    primitiveType,
                    Integer.toString(i),
                    Integer.toString(offset + (i * primitiveType.size())),
                    encodingToken.encoding().byteOrder(),
                    indent));
            }

            sb.append("\n");
            sb.append(indent).append("        return *this;\n");
            sb.append(indent).append("    }\n");
        }

        if (encodingToken.encoding().primitiveType() == PrimitiveType.CHAR)
        {
            new Formatter(sb).format("\n" +
                indent + "    SBE_NODISCARD std::string get%1$sAsString() const\n" +
                indent + "    {\n" +
                "%4$s" +
                "%5$s" +
                indent + "        const char *buffer = m_buffer + m_offset + %2$d;\n" +
                indent + "        std::size_t length = 0;\n\n" +

                indent + "        for (; length < %3$d && *(buffer + length) != '\\0'; ++length);\n" +
                indent + "        std::string result(buffer, length);\n\n" +

                indent + "        return result;\n" +
                indent + "    }\n",
                toUpperFirstChar(propertyName),
                offset,
                arrayLength,
                generateStringNotPresentCondition(propertyToken.version(), indent),
                accessOrderListenerCall);

            generateJsonEscapedStringGetter(sb, encodingToken, indent, propertyName, accessOrderListenerCall);

            new Formatter(sb).format("\n" +
                indent + "    #if __cplusplus >= 201703L\n" +
                indent + "    SBE_NODISCARD std::string_view get%1$sAsStringView() const%6$s\n" +
                indent + "    {\n" +
                "%4$s" +
                "%5$s" +
                indent + "        const char *buffer = m_buffer + m_offset + %2$d;\n" +
                indent + "        std::size_t length = 0;\n\n" +

                indent + "        for (; length < %3$d && *(buffer + length) != '\\0'; ++length);\n" +
                indent + "        std::string_view result(buffer, length);\n\n" +

                indent + "        return result;\n" +
                indent + "    }\n" +
                indent + "    #endif\n",
                toUpperFirstChar(propertyName),
                offset,
                arrayLength,
                generateStringViewNotPresentCondition(propertyToken.version(), indent),
                accessOrderListenerCall,
                noexceptDeclaration);

            new Formatter(sb).format("\n" +
                indent + "    #if __cplusplus >= 201703L\n" +
                indent + "    %1$s &put%2$s(const std::string_view str)\n" +
                indent + "    {\n" +
                indent + "        const std::size_t srcLength = str.length();\n" +
                indent + "        if (srcLength > %4$d)\n" +
                indent + "        {\n" +
                indent + "            throw std::runtime_error(\"string too large for put%2$s [E106]\");\n" +
                indent + "        }\n\n" +

                "%5$s" +
                indent + "        std::memcpy(m_buffer + m_offset + %3$d, str.data(), srcLength);\n" +
                indent + "        for (std::size_t start = srcLength; start < %4$d; ++start)\n" +
                indent + "        {\n" +
                indent + "            m_buffer[m_offset + %3$d + start] = 0;\n" +
                indent + "        }\n\n" +

                indent + "        return *this;\n" +
                indent + "    }\n" +
                indent + "    #else\n" +
                indent + "    %1$s &put%2$s(const std::string &str)\n" +
                indent + "    {\n" +
                indent + "        const std::size_t srcLength = str.length();\n" +
                indent + "        if (srcLength > %4$d)\n" +
                indent + "        {\n" +
                indent + "            throw std::runtime_error(\"string too large for put%2$s [E106]\");\n" +
                indent + "        }\n\n" +

                "%5$s" +
                indent + "        std::memcpy(m_buffer + m_offset + %3$d, str.c_str(), srcLength);\n" +
                indent + "        for (std::size_t start = srcLength; start < %4$d; ++start)\n" +
                indent + "        {\n" +
                indent + "            m_buffer[m_offset + %3$d + start] = 0;\n" +
                indent + "        }\n\n" +

                indent + "        return *this;\n" +
                indent + "    }\n" +
                indent + "    #endif\n",
                containingClassName,
                toUpperFirstChar(propertyName),
                offset,
                arrayLength,
                accessOrderListenerCall);
        }
    }

    private void generateJsonEscapedStringGetter(
        final StringBuilder sb,
        final Token token,
        final String indent,
        final String propertyName,
        final CharSequence accessOrderListenerCall)
    {
        new Formatter(sb).format("\n" +
            indent + "    std::string get%1$sAsJsonEscapedString()\n" +
            indent + "    {\n" +
            "%2$s" +
            "%3$s" +
            indent + "        std::ostringstream oss;\n" +
            indent + "        std::string s = get%1$sAsString();\n\n" +
            indent + "        for (const auto c : s)\n" +
            indent + "        {\n" +
            indent + "            switch (c)\n" +
            indent + "            {\n" +
            indent + "                case '\"': oss << \"\\\\\\\"\"; break;\n" +
            indent + "                case '\\\\': oss << \"\\\\\\\\\"; break;\n" +
            indent + "                case '\\b': oss << \"\\\\b\"; break;\n" +
            indent + "                case '\\f': oss << \"\\\\f\"; break;\n" +
            indent + "                case '\\n': oss << \"\\\\n\"; break;\n" +
            indent + "                case '\\r': oss << \"\\\\r\"; break;\n" +
            indent + "                case '\\t': oss << \"\\\\t\"; break;\n\n" +
            indent + "                default:\n" +
            indent + "                    if ('\\x00' <= c && c <= '\\x1f')\n" +
            indent + "                    {\n" +
            indent + "                        oss << \"\\\\u\"" + " << std::hex << std::setw(4)\n" +
            indent + "                            << std::setfill('0') << (int)(c);\n" +
            indent + "                    }\n" +
            indent + "                    else\n" +
            indent + "                    {\n" +
            indent + "                        oss << c;\n" +
            indent + "                    }\n" +
            indent + "            }\n" +
            indent + "        }\n\n" +
            indent + "        return oss.str();\n" +
            indent + "    }\n",
            toUpperFirstChar(propertyName),
            generateStringNotPresentCondition(token.version(), indent),
            accessOrderListenerCall);
    }

    private void generateConstPropertyMethods(
        final StringBuilder sb, final String propertyName, final Token token, final String indent)
    {
        final String cppTypeName = cppTypeName(token.encoding().primitiveType());

        if (token.encoding().primitiveType() != PrimitiveType.CHAR)
        {
            new Formatter(sb).format("\n" +
                indent + "    SBE_NODISCARD static SBE_CONSTEXPR %1$s %2$s() SBE_NOEXCEPT\n" +
                indent + "    {\n" +
                indent + "        return %3$s;\n" +
                indent + "    }\n",
                cppTypeName,
                propertyName,
                generateLiteral(token.encoding().primitiveType(), token.encoding().constValue().toString()));

            return;
        }

        final byte[] constantValue = token.encoding().constValue().byteArrayValue(token.encoding().primitiveType());
        final StringBuilder values = new StringBuilder();
        for (final byte b : constantValue)
        {
            values.append(b).append(", ");
        }

        if (values.length() > 0)
        {
            values.setLength(values.length() - 2);
        }

        new Formatter(sb).format("\n" +
            indent + "    static SBE_CONSTEXPR std::uint64_t %1$sLength() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %2$d;\n" +
            indent + "    }\n",
            propertyName,
            constantValue.length);

        new Formatter(sb).format("\n" +
            indent + "    SBE_NODISCARD const char *%1$s() const\n" +
            indent + "    {\n" +
            indent + "        static const std::uint8_t %1$sValues[] = { %2$s, 0 };\n\n" +

            indent + "        return (const char *)%1$sValues;\n" +
            indent + "    }\n",
            propertyName,
            values);

        sb.append(String.format("\n" +
            indent + "    SBE_NODISCARD %1$s %2$s(const std::uint64_t index) const\n" +
            indent + "    {\n" +
            indent + "        static const std::uint8_t %2$sValues[] = { %3$s, 0 };\n\n" +

            indent + "        return (char)%2$sValues[index];\n" +
            indent + "    }\n",
            cppTypeName,
            propertyName,
            values));

        new Formatter(sb).format("\n" +
            indent + "    std::uint64_t get%1$s(char *dst, const std::uint64_t length) const\n" +
            indent + "    {\n" +
            indent + "        static std::uint8_t %2$sValues[] = { %3$s };\n" +
            indent + "        std::uint64_t bytesToCopy = " +
            "length < sizeof(%2$sValues) ? length : sizeof(%2$sValues);\n\n" +

            indent + "        std::memcpy(dst, %2$sValues, static_cast<std::size_t>(bytesToCopy));\n" +
            indent + "        return bytesToCopy;\n" +
            indent + "    }\n",
            toUpperFirstChar(propertyName),
            propertyName,
            values);

        new Formatter(sb).format("\n" +
            indent + "    std::string get%1$sAsString() const\n" +
            indent + "    {\n" +
            indent + "        static const std::uint8_t %1$sValues[] = { %2$s };\n\n" +
            indent + "        return std::string((const char *)%1$sValues, %3$s);\n" +
            indent + "    }\n",
            toUpperFirstChar(propertyName),
            values,
            constantValue.length);

        generateJsonEscapedStringGetter(sb, token, indent, propertyName, "");
    }

    private CharSequence generateFixedFlyweightCode(final String className, final int size)
    {
        final String schemaIdType = cppTypeName(ir.headerStructure().schemaIdType());
        final String schemaVersionType = cppTypeName(ir.headerStructure().schemaVersionType());

        return String.format(
            "private:\n" +
            "    char *m_buffer = nullptr;\n" +
            "    std::uint64_t m_bufferLength = 0;\n" +
            "    std::uint64_t m_offset = 0;\n" +
            "    std::uint64_t m_actingVersion = 0;\n\n" +
            "%7$s" +

            "public:\n" +
            "    enum MetaAttribute\n" +
            "    {\n" +
            "        EPOCH, TIME_UNIT, SEMANTIC_TYPE, PRESENCE\n" +
            "    };\n\n" +

            "    union sbe_float_as_uint_u\n" +
            "    {\n" +
            "        float fp_value;\n" +
            "        std::uint32_t uint_value;\n" +
            "    };\n\n" +

            "    union sbe_double_as_uint_u\n" +
            "    {\n" +
            "        double fp_value;\n" +
            "        std::uint64_t uint_value;\n" +
            "    };\n\n" +

            "    %1$s() = default;\n\n" +

            "    %1$s(\n" +
            "        char *buffer,\n" +
            "        const std::uint64_t offset,\n" +
            "        const std::uint64_t bufferLength,\n" +
            "        const std::uint64_t actingVersion) :\n" +
            "        m_buffer(buffer),\n" +
            "        m_bufferLength(bufferLength),\n" +
            "        m_offset(offset),\n" +
            "        m_actingVersion(actingVersion)\n" +
            "    {\n" +
            "        if (SBE_BOUNDS_CHECK_EXPECT(((m_offset + %2$s) > m_bufferLength), false))\n" +
            "        {\n" +
            "            throw std::runtime_error(\"buffer too short for flyweight [E107]\");\n" +
            "        }\n" +
            "    }\n\n" +

            "    %1$s(\n" +
            "        char *buffer,\n" +
            "        const std::uint64_t bufferLength,\n" +
            "        const std::uint64_t actingVersion) :\n" +
            "        %1$s(buffer, 0, bufferLength, actingVersion)\n" +
            "    {\n" +
            "    }\n\n" +

            "    %1$s(\n" +
            "        char *buffer,\n" +
            "        const std::uint64_t bufferLength) :\n" +
            "        %1$s(buffer, 0, bufferLength, sbeSchemaVersion())\n" +
            "    {\n" +
            "    }\n\n" +

            "    %1$s &wrap(\n" +
            "        char *buffer,\n" +
            "        const std::uint64_t offset,\n" +
            "        const std::uint64_t actingVersion,\n" +
            "        const std::uint64_t bufferLength)\n" +
            "    {\n" +
            "        m_buffer = buffer;\n" +
            "        m_bufferLength = bufferLength;\n" +
            "        m_offset = offset;\n" +
            "        m_actingVersion = actingVersion;\n\n" +

            "        if (SBE_BOUNDS_CHECK_EXPECT(((m_offset + %2$s) > m_bufferLength), false))\n" +
            "        {\n" +
            "            throw std::runtime_error(\"buffer too short for flyweight [E107]\");\n" +
            "        }\n\n" +

            "        return *this;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t encodedLength() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return %2$s;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD std::uint64_t offset() const SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return m_offset;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD const char *buffer() const SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return m_buffer;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD char *buffer() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return m_buffer;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD std::uint64_t bufferLength() const SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return m_bufferLength;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD std::uint64_t actingVersion() const SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return m_actingVersion;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD static SBE_CONSTEXPR %3$s sbeSchemaId() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return %4$s;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD static SBE_CONSTEXPR %5$s sbeSchemaVersion() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return %6$s;\n" +
            "    }\n",
            className,
            size,
            schemaIdType,
            generateLiteral(ir.headerStructure().schemaIdType(), Integer.toString(ir.id())),
            schemaVersionType,
            generateLiteral(ir.headerStructure().schemaVersionType(), Integer.toString(ir.version())),
            generateHiddenCopyConstructor("    ", className));
    }

    private static String generateHiddenCopyConstructor(final String indent, final String className)
    {
        final String ctorAndCopyAssignmentDeletion = String.format(
            "#if __cplusplus >= 201103L\n" +
            "%1$s%2$s(const %2$s&) = delete;\n" +
            "%1$s%2$s& operator=(const %2$s&) = delete;\n" +
            "#else\n" +
            "%1$s%2$s(const %2$s&);\n" +
            "%1$s%2$s& operator=(const %2$s&);\n" +
            "#endif\n\n",
            indent, className);

        return DISABLE_IMPLICIT_COPYING ? ctorAndCopyAssignmentDeletion : "";
    }

    private static CharSequence generateConstructorsAndOperators(
        final String className,
        final FieldPrecedenceModel fieldPrecedenceModel)
    {
        final String constructorWithCodecState = null == fieldPrecedenceModel ? "" : String.format(
            "    %1$s(\n" +
            "        char *buffer,\n" +
            "        const std::uint64_t offset,\n" +
            "        const std::uint64_t bufferLength,\n" +
            "        const std::uint64_t actingBlockLength,\n" +
            "        const std::uint64_t actingVersion,\n" +
            "        CodecState codecState) :\n" +
            "        m_buffer(buffer),\n" +
            "        m_bufferLength(bufferLength),\n" +
            "        m_offset(offset),\n" +
            "        m_position(sbeCheckPosition(offset + actingBlockLength)),\n" +
            "        m_actingBlockLength(actingBlockLength),\n" +
            "        m_actingVersion(actingVersion),\n" +
            "        m_codecState(codecState)\n" +
            "    {\n" +
            "    }\n\n",
            className);

        return String.format(
            "    %1$s() = default;\n\n" +

            "%2$s" +

            "    %1$s(\n" +
            "        char *buffer,\n" +
            "        const std::uint64_t offset,\n" +
            "        const std::uint64_t bufferLength,\n" +
            "        const std::uint64_t actingBlockLength,\n" +
            "        const std::uint64_t actingVersion) :\n" +
            "        m_buffer(buffer),\n" +
            "        m_bufferLength(bufferLength),\n" +
            "        m_offset(offset),\n" +
            "        m_position(sbeCheckPosition(offset + actingBlockLength)),\n" +
            "        m_actingBlockLength(actingBlockLength),\n" +
            "        m_actingVersion(actingVersion)\n" +
            "    {\n" +
            "    }\n\n" +

            "    %1$s(char *buffer, const std::uint64_t bufferLength) :\n" +
            "        %1$s(buffer, 0, bufferLength, sbeBlockLength(), sbeSchemaVersion())\n" +
            "    {\n" +
            "    }\n\n" +

            "    %1$s(\n" +
            "        char *buffer,\n" +
            "        const std::uint64_t bufferLength,\n" +
            "        const std::uint64_t actingBlockLength,\n" +
            "        const std::uint64_t actingVersion) :\n" +
            "        %1$s(buffer, 0, bufferLength, actingBlockLength, actingVersion)\n" +
            "    {\n" +
            "    }\n\n",
            className,
            constructorWithCodecState);
    }

    private CharSequence generateMessageFlyweightCode(
        final String className,
        final Token token,
        final FieldPrecedenceModel fieldPrecedenceModel)
    {
        final String blockLengthType = cppTypeName(ir.headerStructure().blockLengthType());
        final String templateIdType = cppTypeName(ir.headerStructure().templateIdType());
        final String schemaIdType = cppTypeName(ir.headerStructure().schemaIdType());
        final String schemaVersionType = cppTypeName(ir.headerStructure().schemaVersionType());
        final String semanticType = token.encoding().semanticType() == null ? "" : token.encoding().semanticType();
        final String headerType = ir.headerStructure().tokens().get(0).name();
        final String semanticVersion = ir.semanticVersion() == null ? "" : ir.semanticVersion();


        final String codecStateArgument = null == fieldPrecedenceModel ? "" : ", m_codecState";

        return String.format(
            "private:\n" +
            "%15$s" +
            "%16$s" +
            "    char *m_buffer = nullptr;\n" +
            "    std::uint64_t m_bufferLength = 0;\n" +
            "    std::uint64_t m_offset = 0;\n" +
            "    std::uint64_t m_position = 0;\n" +
            "    std::uint64_t m_actingBlockLength = 0;\n" +
            "    std::uint64_t m_actingVersion = 0;\n" +
            "%17$s" +

            "    inline std::uint64_t *sbePositionPtr() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return &m_position;\n" +
            "    }\n\n" +

            "%22$s" +

            "public:\n" +
            "    static constexpr %1$s SBE_BLOCK_LENGTH = %2$s;\n" +
            "    static constexpr %3$s SBE_TEMPLATE_ID = %4$s;\n" +
            "    static constexpr %5$s SBE_SCHEMA_ID = %6$s;\n" +
            "    static constexpr %7$s SBE_SCHEMA_VERSION = %8$s;\n" +
            "    static constexpr const char* SBE_SEMANTIC_VERSION = \"%13$s\";\n\n" +

            "    enum MetaAttribute\n" +
            "    {\n" +
            "        EPOCH, TIME_UNIT, SEMANTIC_TYPE, PRESENCE\n" +
            "    };\n\n" +

            "    union sbe_float_as_uint_u\n" +
            "    {\n" +
            "        float fp_value;\n" +
            "        std::uint32_t uint_value;\n" +
            "    };\n\n" +

            "    union sbe_double_as_uint_u\n" +
            "    {\n" +
            "        double fp_value;\n" +
            "        std::uint64_t uint_value;\n" +
            "    };\n\n" +

            "    using messageHeader = %12$s;\n\n" +

            "%18$s" +
            "%11$s" +

            "    SBE_NODISCARD static SBE_CONSTEXPR %1$s sbeBlockLength() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return %2$s;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t sbeBlockAndHeaderLength() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return messageHeader::encodedLength() + sbeBlockLength();\n" +
            "    }\n\n" +

            "    SBE_NODISCARD static SBE_CONSTEXPR %3$s sbeTemplateId() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return %4$s;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD static SBE_CONSTEXPR %5$s sbeSchemaId() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return %6$s;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD static SBE_CONSTEXPR %7$s sbeSchemaVersion() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return %8$s;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD static const char *sbeSemanticVersion() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return \"%13$s\";\n" +
            "    }\n\n" +

            "    SBE_NODISCARD static SBE_CONSTEXPR const char *sbeSemanticType() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return \"%9$s\";\n" +
            "    }\n\n" +

            "    SBE_NODISCARD std::uint64_t offset() const SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return m_offset;\n" +
            "    }\n\n" +

            "    %10$s &wrapForEncode(char *buffer, const std::uint64_t offset, const std::uint64_t bufferLength)\n" +
            "    {\n" +
            "        m_buffer = buffer;\n" +
            "        m_bufferLength = bufferLength;\n" +
            "        m_offset = offset;\n" +
            "        m_actingBlockLength = sbeBlockLength();\n" +
            "        m_actingVersion = sbeSchemaVersion();\n" +
            "        m_position = sbeCheckPosition(m_offset + m_actingBlockLength);\n" +
            "%19$s" +
            "        return *this;\n" +
            "    }\n\n" +

            "    %10$s &wrapAndApplyHeader(" +
            "char *buffer, const std::uint64_t offset, const std::uint64_t bufferLength)\n" +
            "    {\n" +
            "        messageHeader hdr(buffer, offset, bufferLength, sbeSchemaVersion());\n\n" +

            "        hdr\n" +
            "            .blockLength(sbeBlockLength())\n" +
            "            .templateId(sbeTemplateId())\n" +
            "            .schemaId(sbeSchemaId())\n" +
            "            .version(sbeSchemaVersion());\n\n" +

            "        m_buffer = buffer;\n" +
            "        m_bufferLength = bufferLength;\n" +
            "        m_offset = offset + messageHeader::encodedLength();\n" +
            "        m_actingBlockLength = sbeBlockLength();\n" +
            "        m_actingVersion = sbeSchemaVersion();\n" +
            "        m_position = sbeCheckPosition(m_offset + m_actingBlockLength);\n" +
            "%19$s" +
            "        return *this;\n" +
            "    }\n\n" +

            "%20$s" +

            "    %10$s &wrapForDecode(\n" +
            "        char *buffer,\n" +
            "        const std::uint64_t offset,\n" +
            "        const std::uint64_t actingBlockLength,\n" +
            "        const std::uint64_t actingVersion,\n" +
            "        const std::uint64_t bufferLength)\n" +
            "    {\n" +
            "        m_buffer = buffer;\n" +
            "        m_bufferLength = bufferLength;\n" +
            "        m_offset = offset;\n" +
            "        m_actingBlockLength = actingBlockLength;\n" +
            "        m_actingVersion = actingVersion;\n" +
            "        m_position = sbeCheckPosition(m_offset + m_actingBlockLength);\n" +
            "%21$s" +
            "        return *this;\n" +
            "    }\n\n" +

            "    %10$s &sbeRewind()\n" +
            "    {\n" +
            "        return wrapForDecode(" +
            "m_buffer, m_offset, m_actingBlockLength, m_actingVersion, m_bufferLength);\n" +
            "    }\n\n" +

            "    SBE_NODISCARD std::uint64_t sbePosition() const SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return m_position;\n" +
            "    }\n\n" +

            "    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)\n" +
            "    std::uint64_t sbeCheckPosition(const std::uint64_t position)\n" +
            "    {\n" +
            "        if (SBE_BOUNDS_CHECK_EXPECT((position > m_bufferLength), false))\n" +
            "        {\n" +
            "            throw std::runtime_error(\"buffer too short [E100]\");\n" +
            "        }\n" +
            "        return position;\n" +
            "    }\n\n" +

            "    void sbePosition(const std::uint64_t position)\n" +
            "    {\n" +
            "        m_position = sbeCheckPosition(position);\n" +
            "    }\n\n" +

            "    SBE_NODISCARD std::uint64_t encodedLength() const SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return sbePosition() - m_offset;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD std::uint64_t decodeLength() const\n" +
            "    {\n" +
            "        %10$s skipper(m_buffer, m_offset, m_bufferLength, sbeBlockLength(), m_actingVersion%14$s);\n" +
            "        skipper.skip();\n" +
            "        return skipper.encodedLength();\n" +
            "    }\n\n" +

            "    SBE_NODISCARD const char *buffer() const SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return m_buffer;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD char *buffer() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return m_buffer;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD std::uint64_t bufferLength() const SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return m_bufferLength;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD std::uint64_t actingVersion() const SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return m_actingVersion;\n" +
            "    }\n",
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
            generateConstructorsAndOperators(className, fieldPrecedenceModel),
            formatClassName(headerType),
            semanticVersion,
            codecStateArgument,
            generateFieldOrderStateEnum(fieldPrecedenceModel),
            generateLookupTableDeclarations(fieldPrecedenceModel),
            generateFieldOrderStateMember(fieldPrecedenceModel),
            generateAccessOrderErrorType(fieldPrecedenceModel),
            generateEncoderWrapListener(fieldPrecedenceModel),
            generateDecoderWrapListener(fieldPrecedenceModel),
            generateDecoderWrapListenerCall(fieldPrecedenceModel),
            generateHiddenCopyConstructor("    ", className));
    }

    private CharSequence generateAccessOrderErrorType(final FieldPrecedenceModel fieldPrecedenceModel)
    {
        if (null == fieldPrecedenceModel)
        {
            return "";
        }

        final StringBuilder sb = new StringBuilder();
        sb.append(INDENT).append("class AccessOrderError : public std::logic_error\n")
            .append(INDENT).append("{\n")
            .append(INDENT).append("public:\n")
            .append(INDENT).append("    explicit AccessOrderError(const std::string &msg) : std::logic_error(msg) {}\n")
            .append(INDENT).append("};\n\n");
        return sb;
    }

    private static CharSequence generateLookupTableDeclarations(final FieldPrecedenceModel fieldPrecedenceModel)
    {
        if (null == fieldPrecedenceModel)
        {
            return "";
        }

        final StringBuilder sb = new StringBuilder();
        sb.append(INDENT).append("static const std::string STATE_NAME_LOOKUP[")
            .append(fieldPrecedenceModel.stateCount())
            .append("];\n");
        sb.append(INDENT).append("static const std::string STATE_TRANSITIONS_LOOKUP[")
            .append(fieldPrecedenceModel.stateCount())
            .append("];\n\n");

        sb.append(INDENT).append("static std::string codecStateName(CodecState state)\n")
            .append(INDENT).append("{\n")
            .append(TWO_INDENT).append("return STATE_NAME_LOOKUP[static_cast<int>(state)];\n")
            .append(INDENT).append("}\n\n");

        sb.append(INDENT).append("static std::string codecStateTransitions(CodecState state)\n")
            .append(INDENT).append("{\n")
            .append(TWO_INDENT).append("return STATE_TRANSITIONS_LOOKUP[static_cast<int>(state)];\n")
            .append(INDENT).append("}\n\n");

        return sb;
    }

    private static void generateLookupTableDefinitions(
        final StringBuilder sb,
        final String className,
        final FieldPrecedenceModel fieldPrecedenceModel)
    {
        if (null == fieldPrecedenceModel)
        {
            return;
        }

        sb.append("\n").append("const std::string ").append(className).append("::STATE_NAME_LOOKUP[")
            .append(fieldPrecedenceModel.stateCount()).append("] =\n")
            .append("{\n");
        fieldPrecedenceModel.forEachStateOrderedByStateNumber((state) ->
            sb.append(INDENT).append("\"").append(state.name()).append("\",\n"));
        sb.append("};\n\n");

        sb.append("const std::string ").append(className).append("::STATE_TRANSITIONS_LOOKUP[")
            .append(fieldPrecedenceModel.stateCount()).append("] =\n")
            .append("{\n");
        fieldPrecedenceModel.forEachStateOrderedByStateNumber((state) ->
        {
            sb.append(INDENT).append("\"");
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
        sb.append("};\n\n");
    }

    private static CharSequence qualifiedStateCase(final FieldPrecedenceModel.State state)
    {
        return "CodecState::" + state.name();
    }

    private static CharSequence stateCaseForSwitchCase(final FieldPrecedenceModel.State state)
    {
        return qualifiedStateCase(state);
    }

    private static CharSequence unqualifiedStateCase(final FieldPrecedenceModel.State state)
    {
        return state.name();
    }

    private static CharSequence generateFieldOrderStateEnum(final FieldPrecedenceModel fieldPrecedenceModel)
    {
        if (null == fieldPrecedenceModel)
        {
            return "";
        }

        final StringBuilder sb = new StringBuilder();

        sb.append("    /**\n");
        sb.append("     * The states in which a encoder/decoder/codec can live.\n");
        sb.append("     *\n");
        sb.append("     * <p>The state machine diagram below, encoded in the dot language, describes\n");
        sb.append("     * the valid state transitions according to the order in which fields may be\n");
        sb.append("     * accessed safely. Tools such as PlantUML and Graphviz can render it.\n");
        sb.append("     *\n");
        sb.append("     * <pre>{@code\n");
        fieldPrecedenceModel.generateGraph(sb, "     *   ");
        sb.append("     * }</pre>\n");
        sb.append("     */\n");
        sb.append(INDENT).append("enum class CodecState\n")
            .append(INDENT).append("{\n");
        fieldPrecedenceModel.forEachStateOrderedByStateNumber((state) ->
            sb.append(INDENT).append(INDENT).append(unqualifiedStateCase(state))
            .append(" = ").append(state.number())
            .append(",\n"));
        sb.append(INDENT).append("};\n\n");

        return sb;
    }

    private static CharSequence generateFieldOrderStateMember(final FieldPrecedenceModel fieldPrecedenceModel)
    {
        if (null == fieldPrecedenceModel)
        {
            return "\n";
        }

        final StringBuilder sb = new StringBuilder();

        sb.append(INDENT).append("CodecState m_codecState = ")
            .append(qualifiedStateCase(fieldPrecedenceModel.notWrappedState()))
            .append(";\n\n");

        sb.append(INDENT).append("CodecState codecState() const\n")
            .append(INDENT).append("{\n")
            .append(INDENT).append(INDENT).append("return m_codecState;\n")
            .append(INDENT).append("}\n\n");

        sb.append(INDENT).append("CodecState *codecStatePtr()\n")
            .append(INDENT).append("{\n")
            .append(INDENT).append(INDENT).append("return &m_codecState;\n")
            .append(INDENT).append("}\n\n");

        sb.append(INDENT).append("void codecState(CodecState newState)\n")
            .append(INDENT).append("{\n")
            .append(INDENT).append(INDENT).append("m_codecState = newState;\n")
            .append(INDENT).append("}\n\n");

        return sb;
    }

    private static CharSequence generateDecoderWrapListener(final FieldPrecedenceModel fieldPrecedenceModel)
    {
        if (null == fieldPrecedenceModel)
        {
            return "";
        }

        if (fieldPrecedenceModel.versionCount() == 1)
        {
            return "";
        }

        final StringBuilder sb = new StringBuilder();
        sb.append(INDENT).append("void onWrapForDecode(std::uint64_t actingVersion)\n")
            .append(INDENT).append("{\n")
            .append(INDENT).append(INDENT).append("switch(actingVersion)\n")
            .append(INDENT).append(INDENT).append("{\n");

        fieldPrecedenceModel.forEachWrappedStateByVersion((version, state) ->
            sb.append(INDENT).append(TWO_INDENT).append("case ").append(version).append(":\n")
            .append(INDENT).append(THREE_INDENT).append("codecState(")
            .append(qualifiedStateCase(state)).append(");\n")
            .append(INDENT).append(THREE_INDENT).append("break;\n"));

        sb.append(INDENT).append(TWO_INDENT).append("default:\n")
            .append(INDENT).append(THREE_INDENT).append("codecState(")
            .append(qualifiedStateCase(fieldPrecedenceModel.latestVersionWrappedState())).append(");\n")
            .append(INDENT).append(THREE_INDENT).append("break;\n")
            .append(INDENT).append(INDENT).append("}\n")
            .append(INDENT).append("}\n\n");

        return sb;
    }


    private CharSequence generateDecoderWrapListenerCall(final FieldPrecedenceModel fieldPrecedenceModel)
    {
        if (null == fieldPrecedenceModel)
        {
            return "";
        }

        if (fieldPrecedenceModel.versionCount() == 1)
        {
            final StringBuilder sb = new StringBuilder();
            sb.append("#if defined(").append(precedenceChecksFlagName).append(")\n")
                .append(TWO_INDENT).append("codecState(")
                .append(qualifiedStateCase(fieldPrecedenceModel.latestVersionWrappedState())).append(");\n")
                .append("#endif\n");
            return sb;
        }

        return generateAccessOrderListenerCall(fieldPrecedenceModel, TWO_INDENT, "onWrapForDecode", "actingVersion");
    }

    private CharSequence generateEncoderWrapListener(final FieldPrecedenceModel fieldPrecedenceModel)
    {
        if (null == fieldPrecedenceModel)
        {
            return "";
        }

        final StringBuilder sb = new StringBuilder();
        sb.append("#if defined(").append(precedenceChecksFlagName).append(")\n")
            .append(TWO_INDENT).append("codecState(")
            .append(qualifiedStateCase(fieldPrecedenceModel.latestVersionWrappedState()))
            .append(");\n")
            .append("#endif\n");
        return sb;
    }

    private void generateFields(
        final StringBuilder sb,
        final String containingClassName,
        final List<Token> tokens,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent)
    {
        for (int i = 0, size = tokens.size(); i < size; i++)
        {
            final Token signalToken = tokens.get(i);
            if (signalToken.signal() == Signal.BEGIN_FIELD)
            {
                final Token encodingToken = tokens.get(i + 1);
                final String propertyName = formatPropertyName(signalToken.name());

                generateFieldMetaAttributeMethod(sb, signalToken, indent);
                generateFieldCommonMethods(indent, sb, signalToken, encodingToken, propertyName);

                generateAccessOrderListenerMethod(sb, fieldPrecedenceModel, indent, signalToken);

                switch (encodingToken.signal())
                {
                    case ENCODING:
                        generatePrimitiveProperty(
                            sb, containingClassName, propertyName, signalToken, encodingToken,
                            fieldPrecedenceModel, indent);
                        break;

                    case BEGIN_ENUM:
                        generateEnumProperty(sb, containingClassName, signalToken, propertyName, encodingToken,
                            fieldPrecedenceModel, indent);
                        break;

                    case BEGIN_SET:
                        generateBitsetProperty(sb, propertyName, signalToken, encodingToken,
                            fieldPrecedenceModel, indent);
                        break;

                    case BEGIN_COMPOSITE:
                        generateCompositeProperty(sb, propertyName, signalToken, encodingToken,
                            fieldPrecedenceModel, indent);
                        break;

                    default:
                        break;
                }
            }
        }
    }

    private void generateFieldCommonMethods(
        final String indent,
        final StringBuilder sb,
        final Token fieldToken,
        final Token encodingToken,
        final String propertyName)
    {
        new Formatter(sb).format("\n" +
            indent + "    static SBE_CONSTEXPR std::uint16_t %1$sId() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %2$d;\n" +
            indent + "    }\n",
            propertyName,
            fieldToken.id());

        final int version = fieldToken.version();
        final String versionCheck = 0 == version ?
            "        return true;\n" : "        return m_actingVersion >= %1$sSinceVersion();\n";
        new Formatter(sb).format("\n" +
            indent + "    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t %1$sSinceVersion() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %2$d;\n" +
            indent + "    }\n\n" +

            indent + "    SBE_NODISCARD bool %1$sInActingVersion() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + versionCheck +
            indent + "    }\n",
            propertyName,
            version);

        new Formatter(sb).format("\n" +
            indent + "    SBE_NODISCARD static SBE_CONSTEXPR std::size_t %1$sEncodingOffset() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %2$d;\n" +
            indent + "    }\n",
            propertyName,
            encodingToken.offset());
    }

    private static void generateFieldMetaAttributeMethod(
        final StringBuilder sb, final Token token, final String indent)
    {
        final Encoding encoding = token.encoding();
        final String epoch = encoding.epoch() == null ? "" : encoding.epoch();
        final String timeUnit = encoding.timeUnit() == null ? "" : encoding.timeUnit();
        final String semanticType = encoding.semanticType() == null ? "" : encoding.semanticType();

        sb.append("\n")
            .append(indent).append("    SBE_NODISCARD static const char *")
            .append(token.name()).append("MetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT\n")
            .append(indent).append("    {\n")
            .append(indent).append("        switch (metaAttribute)\n")
            .append(indent).append("        {\n");

        if (!Strings.isEmpty(epoch))
        {
            sb.append(indent)
                .append("            case MetaAttribute::EPOCH: return \"").append(epoch).append("\";\n");
        }

        if (!Strings.isEmpty(timeUnit))
        {
            sb.append(indent)
                .append("            case MetaAttribute::TIME_UNIT: return \"").append(timeUnit).append("\";\n");
        }

        if (!Strings.isEmpty(semanticType))
        {
            sb.append(indent)
                .append("            case MetaAttribute::SEMANTIC_TYPE: return \"").append(semanticType)
                .append("\";\n");
        }

        sb
            .append(indent).append("            case MetaAttribute::PRESENCE: return \"")
            .append(encoding.presence().toString().toLowerCase()).append("\";\n")
            .append(indent).append("            default: return \"\";\n")
            .append(indent).append("        }\n")
            .append(indent).append("    }\n");
    }

    private static CharSequence generateEnumFieldNotPresentCondition(
        final int sinceVersion, final String enumName, final String indent)
    {
        if (0 == sinceVersion)
        {
            return "";
        }

        return String.format(
            indent + "        if (m_actingVersion < %1$d)\n" +
            indent + "        {\n" +
            indent + "            return %2$s::NULL_VALUE;\n" +
            indent + "        }\n\n",
            sinceVersion,
            enumName);
    }

    private void generateEnumProperty(
        final StringBuilder sb,
        final String containingClassName,
        final Token fieldToken,
        final String propertyName,
        final Token encodingToken,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent)
    {
        final String enumName = formatClassName(encodingToken.applicableTypeName());
        final PrimitiveType primitiveType = encodingToken.encoding().primitiveType();
        final String typeName = cppTypeName(primitiveType);
        final int offset = encodingToken.offset();

        new Formatter(sb).format("\n" +
            indent + "    SBE_NODISCARD static SBE_CONSTEXPR std::size_t %1$sEncodingLength() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %2$d;\n" +
            indent + "    }\n",
            propertyName,
            fieldToken.encodedLength());

        if (fieldToken.isConstantEncoding())
        {
            final String constValue = fieldToken.encoding().constValue().toString();

            new Formatter(sb).format("\n" +
                indent + "    SBE_NODISCARD static SBE_CONSTEXPR %1$s::Value %2$sConstValue() SBE_NOEXCEPT\n" +
                indent + "    {\n" +
                indent + "        return %1$s::Value::%3$s;\n" +
                indent + "    }\n",
                enumName,
                propertyName,
                constValue.substring(constValue.indexOf(".") + 1));

            new Formatter(sb).format("\n" +
                indent + "    SBE_NODISCARD %1$s::Value %2$s() const\n" +
                indent + "    {\n" +
                "%3$s" +
                indent + "        return %1$s::Value::%4$s;\n" +
                indent + "    }\n",
                enumName,
                propertyName,
                generateEnumFieldNotPresentCondition(fieldToken.version(), enumName, indent),
                constValue.substring(constValue.indexOf(".") + 1));

            new Formatter(sb).format("\n" +
                indent + "    SBE_NODISCARD %1$s %2$sRaw() const SBE_NOEXCEPT\n" +
                indent + "    {\n" +
                indent + "        return static_cast<%1$s>(%3$s::Value::%4$s);\n" +
                indent + "    }\n",
                typeName,
                propertyName,
                enumName,
                constValue.substring(constValue.indexOf(".") + 1));
        }
        else
        {
            final String offsetStr = Integer.toString(offset);

            final CharSequence accessOrderListenerCall = generateAccessOrderListenerCall(
                fieldPrecedenceModel, indent + TWO_INDENT, fieldToken);

            final String noexceptDeclaration = noexceptDeclaration(fieldPrecedenceModel);

            new Formatter(sb).format("\n" +
                indent + "    SBE_NODISCARD %1$s %2$sRaw() const%6$s\n" +
                indent + "    {\n" +
                "%3$s" +
                "%4$s" +
                "%5$s" +
                indent + "    }\n",
                typeName,
                propertyName,
                generateFieldNotPresentCondition(fieldToken.version(), encodingToken.encoding(), indent),
                accessOrderListenerCall,
                generateLoadValue(primitiveType, offsetStr, encodingToken.encoding().byteOrder(), indent),
                noexceptDeclaration);

            new Formatter(sb).format("\n" +
                indent + "    SBE_NODISCARD %1$s::Value %2$s() const\n" +
                indent + "    {\n" +
                "%3$s" +
                "%7$s" +
                indent + "        %5$s val;\n" +
                indent + "        std::memcpy(&val, m_buffer + m_offset + %6$d, sizeof(%5$s));\n" +
                indent + "        return %1$s::get(%4$s(val));\n" +
                indent + "    }\n",
                enumName,
                propertyName,
                generateEnumFieldNotPresentCondition(fieldToken.version(), enumName, indent),
                formatByteOrderEncoding(encodingToken.encoding().byteOrder(), primitiveType),
                typeName,
                offset,
                accessOrderListenerCall);

            new Formatter(sb).format("\n" +
                indent + "    %1$s &%2$s(const %3$s::Value value)%8$s\n" +
                indent + "    {\n" +
                "%7$s" +
                indent + "        %4$s val = %6$s(value);\n" +
                indent + "        std::memcpy(m_buffer + m_offset + %5$d, &val, sizeof(%4$s));\n" +
                indent + "        return *this;\n" +
                indent + "    }\n",
                formatClassName(containingClassName),
                propertyName,
                enumName,
                typeName,
                offset,
                formatByteOrderEncoding(encodingToken.encoding().byteOrder(), primitiveType),
                accessOrderListenerCall,
                noexceptDeclaration);
        }
    }

    private void generateBitsetProperty(
        final StringBuilder sb,
        final String propertyName,
        final Token fieldToken,
        final Token encodingToken,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent)
    {
        final String bitsetName = formatClassName(encodingToken.applicableTypeName());
        final int offset = encodingToken.offset();

        new Formatter(sb).format("\n" +
            indent + "private:\n" +
            indent + "    %1$s m_%2$s;\n\n" +

            indent + "public:\n",
            bitsetName,
            propertyName);

        final CharSequence accessOrderListenerCall = generateAccessOrderListenerCall(
            fieldPrecedenceModel, indent + TWO_INDENT, fieldToken);

        new Formatter(sb).format(
            indent + "    SBE_NODISCARD %1$s &%2$s()\n" +
            indent + "    {\n" +
            "%4$s" +
            indent + "        m_%2$s.wrap(m_buffer, m_offset + %3$d, m_actingVersion, m_bufferLength);\n" +
            indent + "        return m_%2$s;\n" +
            indent + "    }\n",
            bitsetName,
            propertyName,
            offset,
            accessOrderListenerCall);

        new Formatter(sb).format("\n" +
            indent + "    static SBE_CONSTEXPR std::size_t %1$sEncodingLength() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %2$d;\n" +
            indent + "    }\n",
            propertyName,
            encodingToken.encoding().primitiveType().size());
    }

    private void generateCompositeProperty(
        final StringBuilder sb,
        final String propertyName,
        final Token fieldToken,
        final Token encodingToken,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String indent)
    {
        final String compositeName = formatClassName(encodingToken.applicableTypeName());

        new Formatter(sb).format("\n" +
            "private:\n" +
            indent + "    %1$s m_%2$s;\n\n" +

            "public:\n",
            compositeName,
            propertyName);

        final CharSequence accessOrderListenerCall = generateAccessOrderListenerCall(
            fieldPrecedenceModel, indent + TWO_INDENT, fieldToken);

        new Formatter(sb).format(
            indent + "    SBE_NODISCARD %1$s &%2$s()\n" +
            indent + "    {\n" +
            "%4$s" +
            indent + "        m_%2$s.wrap(m_buffer, m_offset + %3$d, m_actingVersion, m_bufferLength);\n" +
            indent + "        return m_%2$s;\n" +
            indent + "    }\n",
            compositeName,
            propertyName,
            encodingToken.offset(),
            accessOrderListenerCall);
    }

    private CharSequence generateNullValueLiteral(final PrimitiveType primitiveType, final Encoding encoding)
    {
        // Visual C++ does not handle minimum integer values properly
        // See: http://msdn.microsoft.com/en-us/library/4kh09110.aspx
        // So null values get special handling
        if (null == encoding.nullValue())
        {
            switch (primitiveType)
            {
                case CHAR:
                case FLOAT:
                case DOUBLE:
                    break; // no special handling
                case INT8:
                    return "SBE_NULLVALUE_INT8";
                case INT16:
                    return "SBE_NULLVALUE_INT16";
                case INT32:
                    return "SBE_NULLVALUE_INT32";
                case INT64:
                    return "SBE_NULLVALUE_INT64";
                case UINT8:
                    return "SBE_NULLVALUE_UINT8";
                case UINT16:
                    return "SBE_NULLVALUE_UINT16";
                case UINT32:
                    return "SBE_NULLVALUE_UINT32";
                case UINT64:
                    return "SBE_NULLVALUE_UINT64";
            }
        }

        return generateLiteral(primitiveType, encoding.applicableNullValue().toString());
    }

    private static CharSequence generateLiteral(final PrimitiveType type, final String value)
    {
        String literal = "";

        switch (type)
        {
            case CHAR:
            case UINT8:
            case UINT16:
            case INT8:
            case INT16:
                literal = "static_cast<" + cppTypeName(type) + ">(" + value + ")";
                break;

            case UINT32:
                literal = "UINT32_C(0x" + Integer.toHexString((int)Long.parseLong(value)) + ")";
                break;

            case INT32:
                final long intValue = Long.parseLong(value);
                if (intValue == Integer.MIN_VALUE)
                {
                    literal = "INT32_MIN";
                }
                else
                {
                    literal = "INT32_C(" + value + ")";
                }
                break;

            case FLOAT:
                literal = value.endsWith("NaN") ? "SBE_FLOAT_NAN" : value + "f";
                break;

            case INT64:
                final long longValue = Long.parseLong(value);
                if (longValue == Long.MIN_VALUE)
                {
                    literal = "INT64_MIN";
                }
                else
                {
                    literal = "INT64_C(" + value + ")";
                }
                break;

            case UINT64:
                literal = "UINT64_C(0x" + Long.toHexString(Long.parseLong(value)) + ")";
                break;

            case DOUBLE:
                literal = value.endsWith("NaN") ? "SBE_DOUBLE_NAN" : value;
                break;
        }

        return literal;
    }

    private void generateDisplay(
        final StringBuilder sb,
        final String name,
        final List<Token> fields,
        final List<Token> groups,
        final List<Token> varData)
    {
        new Formatter(sb).format("\n" +
            "template<typename CharT, typename Traits>\n" +
            "friend std::basic_ostream<CharT, Traits> & operator << (\n" +
            "    std::basic_ostream<CharT, Traits> &builder, const %1$s &_writer)\n" +
            "{\n" +
            "    %1$s writer(\n" +
            "        _writer.m_buffer,\n" +
            "        _writer.m_offset,\n" +
            "        _writer.m_bufferLength,\n" +
            "        _writer.m_actingBlockLength,\n" +
            "        _writer.m_actingVersion);\n\n" +
            "    builder << '{';\n" +
            "    builder << R\"(\"Name\": \"%1$s\", )\";\n" +
            "    builder << R\"(\"sbeTemplateId\": )\";\n" +
            "    builder << writer.sbeTemplateId();\n" +
            "    builder << \", \";\n\n" +
            "%2$s" +
            "    builder << '}';\n\n" +
            "    return builder;\n" +
            "}\n",
            formatClassName(name),
            appendDisplay(fields, groups, varData, INDENT));
    }

    private CharSequence generateGroupDisplay(
        final String name,
        final List<Token> fields,
        final List<Token> groups,
        final List<Token> varData,
        final String indent)
    {
        return String.format("\n" +
            indent + "template<typename CharT, typename Traits>\n" +
            indent + "friend std::basic_ostream<CharT, Traits> & operator << (\n" +
            indent + "    std::basic_ostream<CharT, Traits> &builder, %1$s &writer)\n" +
            indent + "{\n" +
            indent + "    builder << '{';\n" +
            "%2$s" +
            indent + "    builder << '}';\n\n" +
            indent + "    return builder;\n" +
            indent + "}\n",
            formatClassName(name),
            appendDisplay(fields, groups, varData, indent + INDENT));
    }

    private CharSequence generateCompositeDisplay(final String name, final List<Token> tokens)
    {
        return String.format("\n" +
            "template<typename CharT, typename Traits>\n" +
            "friend std::basic_ostream<CharT, Traits> & operator << (\n" +
            "    std::basic_ostream<CharT, Traits> &builder, %1$s &writer)\n" +
            "{\n" +
            "    builder << '{';\n" +
            "%2$s" +
            "    builder << '}';\n\n" +
            "    return builder;\n" +
            "}\n\n",
            formatClassName(name),
            appendDisplay(tokens, new ArrayList<>(), new ArrayList<>(), INDENT));
    }

    private CharSequence appendDisplay(
        final List<Token> fields, final List<Token> groups, final List<Token> varData, final String indent)
    {
        final StringBuilder sb = new StringBuilder();
        final boolean[] atLeastOne = { false };

        for (int i = 0, size = fields.size(); i < size;)
        {
            final Token fieldToken = fields.get(i);
            final Token encodingToken = fields.get(fieldToken.signal() == Signal.BEGIN_FIELD ? i + 1 : i);

            writeTokenDisplay(sb, fieldToken.name(), encodingToken, atLeastOne, indent);
            i += fieldToken.componentTokenCount();
        }

        for (int i = 0, size = groups.size(); i < size; i++)
        {
            final Token groupToken = groups.get(i);
            if (groupToken.signal() != Signal.BEGIN_GROUP)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_GROUP: token=" + groupToken);
            }

            if (atLeastOne[0])
            {
                sb.append(indent).append("builder << \", \";\n");
            }
            atLeastOne[0] = true;

            new Formatter(sb).format(
                indent + "{\n" +
                indent + "    bool atLeastOne = false;\n" +
                indent + "    builder << R\"(\"%3$s\": [)\";\n" +
                indent + "    writer.%2$s().forEach(\n" +
                indent + "        [&](%1$s &%2$s)\n" +
                indent + "        {\n" +
                indent + "            if (atLeastOne)\n" +
                indent + "            {\n" +
                indent + "                builder << \", \";\n" +
                indent + "            }\n" +
                indent + "            atLeastOne = true;\n" +
                indent + "            builder << %2$s;\n" +
                indent + "        });\n" +
                indent + "    builder << ']';\n" +
                indent + "}\n\n",
                formatClassName(groupToken.name()),
                formatPropertyName(groupToken.name()),
                groupToken.name());

            i = findEndSignal(groups, i, Signal.END_GROUP, groupToken.name());
        }

        for (int i = 0, size = varData.size(); i < size;)
        {
            final Token varDataToken = varData.get(i);
            if (varDataToken.signal() != Signal.BEGIN_VAR_DATA)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_VAR_DATA: token=" + varDataToken);
            }

            if (atLeastOne[0])
            {
                sb.append(indent).append("builder << \", \";\n");
            }
            atLeastOne[0] = true;

            final String characterEncoding = varData.get(i + 3).encoding().characterEncoding();
            sb.append(indent).append("builder << R\"(\"").append(varDataToken.name()).append("\": )\";\n");

            if (null == characterEncoding)
            {
                final String skipFunction = "writer.skip" + toUpperFirstChar(varDataToken.name()) + "()";

                sb.append(indent).append("builder << '\"' <<\n").append(indent).append(INDENT).append(skipFunction)
                    .append(" << \" bytes of raw data\\\"\";\n");
            }
            else
            {
                final String getAsStringFunction =
                    "writer.get" + toUpperFirstChar(varDataToken.name()) + "AsJsonEscapedString().c_str()";

                sb.append(indent).append("builder << '\"' <<\n").append(indent).append(INDENT)
                    .append(getAsStringFunction).append(" << '\"';\n\n");
            }

            i += varDataToken.componentTokenCount();
        }

        return sb;
    }

    private void writeTokenDisplay(
        final StringBuilder sb,
        final String fieldTokenName,
        final Token typeToken,
        final boolean[] atLeastOne,
        final String indent)
    {
        if (typeToken.encodedLength() <= 0 || typeToken.isConstantEncoding())
        {
            return;
        }

        if (atLeastOne[0])
        {
            sb.append(indent).append("builder << \", \";\n");
        }
        else
        {
            atLeastOne[0] = true;
        }

        sb.append(indent).append("builder << R\"(\"").append(fieldTokenName).append("\": )\";\n");
        final String fieldName = "writer." + formatPropertyName(fieldTokenName);

        switch (typeToken.signal())
        {
            case ENCODING:
                if (typeToken.arrayLength() > 1)
                {
                    if (typeToken.encoding().primitiveType() == PrimitiveType.CHAR)
                    {
                        final String getAsStringFunction =
                            "writer.get" + toUpperFirstChar(fieldTokenName) + "AsJsonEscapedString().c_str()";

                        sb.append(indent).append("builder << '\"' <<\n").append(indent).append(INDENT)
                            .append(getAsStringFunction).append(" << '\"';\n");
                    }
                    else
                    {
                        sb.append(
                            indent + "builder << '[';\n" +
                            indent + "for (std::size_t i = 0, length = " + fieldName + "Length(); i < length; i++)\n" +
                            indent + "{\n" +
                            indent + "    if (i)\n" +
                            indent + "    {\n" +
                            indent + "        builder << ',';\n" +
                            indent + "    }\n" +
                            indent + "    builder << +" + fieldName + "(i);\n" +
                            indent + "}\n" +
                            indent + "builder << ']';\n");
                    }
                }
                else
                {
                    if (typeToken.encoding().primitiveType() == PrimitiveType.CHAR)
                    {
                        sb.append(
                            indent + "if (std::isprint(" + fieldName + "()))\n" +
                            indent + "{\n" +
                            indent + "    builder << '\"' << (char)" + fieldName + "() << '\"';\n" +
                            indent + "}\n" +
                            indent + "else\n" +
                            indent + "{\n" +
                            indent + "    builder << (int)" + fieldName + "();\n" +
                            indent + "}\n");
                    }
                    else
                    {
                        sb.append(indent).append("builder << +").append(fieldName).append("();\n");
                    }
                }
                break;

            case BEGIN_ENUM:
                sb.append(indent).append("builder << '\"' << ").append(fieldName).append("() << '\"';\n");
                break;

            case BEGIN_SET:
            case BEGIN_COMPOSITE:
                if (0 == typeToken.version())
                {
                    sb.append(indent).append("builder << ").append(fieldName).append("();\n");
                }
                else
                {
                    new Formatter(sb).format(
                        indent + "if (%1$sInActingVersion())\n" +
                        indent + "{\n" +
                        indent + "    builder << %1$s();\n" +
                        indent + "}\n" +
                        indent + "else\n" +
                        indent + "{\n" +
                        indent + "    builder << %2$s;\n" +
                        indent + "}\n",
                        fieldName,
                        typeToken.signal() == Signal.BEGIN_SET ? "\"[]\"" : "\"{}\"");
                }
                break;

            default:
                break;
        }

        sb.append('\n');
    }

    private CharSequence generateChoicesDisplay(final String name, final List<Token> tokens)
    {
        final String indent = INDENT;
        final StringBuilder sb = new StringBuilder();
        final List<Token> choiceTokens = new ArrayList<>();

        collect(Signal.CHOICE, tokens, 0, choiceTokens);

        new Formatter(sb).format("\n" +
            indent + "template<typename CharT, typename Traits>\n" +
            indent + "friend std::basic_ostream<CharT, Traits> & operator << (\n" +
            indent + "    std::basic_ostream<CharT, Traits> &builder, %1$s &writer)\n" +
            indent + "{\n" +
            indent + "    builder << '[';\n",
            name);

        if (choiceTokens.size() > 1)
        {
            sb.append(indent + "    bool atLeastOne = false;\n");
        }

        for (int i = 0, size = choiceTokens.size(); i < size; i++)
        {
            final Token token = choiceTokens.get(i);
            final String choiceName = "writer." + formatPropertyName(token.name());

            sb.append(indent + "    if (").append(choiceName).append("())\n")
                .append(indent).append("    {\n");

            if (i > 0)
            {
                sb.append(
                    indent + "        if (atLeastOne)\n" +
                    indent + "        {\n" +
                    indent + "            builder << \",\";\n" +
                    indent + "        }\n");
            }
            sb.append(indent + "        builder << R\"(\"").append(formatPropertyName(token.name())).append("\")\";\n");

            if (i < (size - 1))
            {
                sb.append(indent + "        atLeastOne = true;\n");
            }

            sb.append(indent + "    }\n");
        }

        sb.append(
            indent + "    builder << ']';\n" +
            indent + "    return builder;\n" +
            indent + "}\n");

        return sb;
    }

    private CharSequence generateEnumDisplay(final List<Token> tokens, final Token encodingToken)
    {
        final String enumName = formatClassName(encodingToken.applicableTypeName());
        final StringBuilder sb = new StringBuilder();

        new Formatter(sb).format("\n" +
            "    static const char *c_str(const %1$s::Value value)\n" +
            "    {\n" +
            "        switch (value)\n" +
            "        {\n",
            enumName);

        for (final Token token : tokens)
        {
            new Formatter(sb).format(
                "            case %1$s: return \"%1$s\";\n",
                token.name());
        }

        sb.append("            case NULL_VALUE: return \"NULL_VALUE\";\n").append("        }\n\n");

        if (shouldDecodeUnknownEnumValues)
        {
            sb.append("        return \"SBE_UNKNOWN\";\n").append("    }\n\n");
        }
        else
        {
            new Formatter(sb).format(
                "        throw std::runtime_error(\"unknown value for enum %1$s [E103]:\");\n" +
                "    }\n\n",
                enumName);
        }

        new Formatter(sb).format(
            "    template<typename CharT, typename Traits>\n" +
            "    friend std::basic_ostream<CharT, Traits> & operator << (\n" +
            "        std::basic_ostream<CharT, Traits> &os, %1$s::Value m)\n" +
            "    {\n" +
            "        return os << %1$s::c_str(m);\n" +
            "    }\n",
            enumName);

        return sb;
    }

    private Object[] generateMessageLengthArgs(
        final List<Token> groups,
        final List<Token> varData,
        final String indent,
        final boolean withName)
    {
        final StringBuilder sb = new StringBuilder();
        int count = 0;

        for (int i = 0, size = groups.size(); i < size; i++)
        {
            final Token groupToken = groups.get(i);
            if (groupToken.signal() != Signal.BEGIN_GROUP)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_GROUP: token=" + groupToken);
            }

            final int endSignal = findEndSignal(groups, i, Signal.END_GROUP, groupToken.name());
            final String groupName = formatPropertyName(groupToken.name());

            if (count > 0)
            {
                sb.append(",\n").append(indent);
            }

            final List<Token> thisGroup = groups.subList(i, endSignal + 1);

            if (isMessageConstLength(thisGroup))
            {
                sb.append("std::size_t");
                if (withName)
                {
                    sb.append(" ").append(groupName).append("Length = 0");
                }
            }
            else
            {
                sb.append("const std::vector<std::tuple<");
                sb.append(generateMessageLengthArgs(thisGroup, indent + INDENT, false)[0]);
                sb.append(">> &");

                if (withName)
                {
                    sb.append(groupName).append("ItemLengths = {}");
                }
            }

            count += 1;

            i = endSignal;
        }

        for (int i = 0, size = varData.size(); i < size;)
        {
            final Token varDataToken = varData.get(i);
            if (varDataToken.signal() != Signal.BEGIN_VAR_DATA)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_VAR_DATA: token=" + varDataToken);
            }

            if (count > 0)
            {
                sb.append(",\n").append(indent);
            }

            sb.append("std::size_t");
            if (withName)
            {
                sb.append(" ").append(formatPropertyName(varDataToken.name())).append("Length = 0");
            }

            count += 1;

            i += varDataToken.componentTokenCount();
        }

        CharSequence result = sb;
        if (count > 1)
        {
            result = "\n" + indent + result;
        }

        return new Object[]{ result, count };
    }

    private Object[] generateMessageLengthArgs(final List<Token> tokens, final String indent, final boolean withName)
    {
        int i = 0;

        final Token groupToken = tokens.get(i);
        if (groupToken.signal() != Signal.BEGIN_GROUP)
        {
            throw new IllegalStateException("tokens must begin with BEGIN_GROUP: token=" + groupToken);
        }

        ++i;
        final int groupHeaderTokenCount = tokens.get(i).componentTokenCount();
        i += groupHeaderTokenCount;

        final List<Token> fields = new ArrayList<>();
        i = collectFields(tokens, i, fields);

        final List<Token> groups = new ArrayList<>();
        i = collectGroups(tokens, i, groups);

        final List<Token> varData = new ArrayList<>();
        collectVarData(tokens, i, varData);

        return generateMessageLengthArgs(groups, varData, indent, withName);
    }

    private boolean isMessageConstLength(final List<Token> tokens)
    {
        final Integer count = (Integer)generateMessageLengthArgs(tokens, BASE_INDENT, false)[1];

        return count == 0;
    }

    private CharSequence generateMessageLengthCallPre17Helper(final List<Token> tokens)
    {
        final StringBuilder sb = new StringBuilder();
        final Integer count = (Integer)generateMessageLengthArgs(tokens, BASE_INDENT, false)[1];

        for (int i = 0; i < count; i++)
        {
            if (i > 0)
            {
                sb.append(", ");
            }

            sb.append("std::get<").append(i).append(">(e)");
        }

        return sb;
    }

    private CharSequence generateMessageLength(final List<Token> groups, final List<Token> varData, final String indent)
    {
        final StringBuilder sbEncode = new StringBuilder();
        final StringBuilder sbSkip = new StringBuilder();

        for (int i = 0, size = groups.size(); i < size; i++)
        {
            final Token groupToken = groups.get(i);

            if (groupToken.signal() != Signal.BEGIN_GROUP)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_GROUP: token=" + groupToken);
            }

            final int endSignal = findEndSignal(groups, i, Signal.END_GROUP, groupToken.name());
            final List<Token> thisGroup = groups.subList(i, endSignal + 1);

            final Token numInGroupToken = Generators.findFirst("numInGroup", groups, i);
            final long minCount = numInGroupToken.encoding().applicableMinValue().longValue();
            final long maxCount = numInGroupToken.encoding().applicableMaxValue().longValue();

            final String countName = formatPropertyName(groupToken.name()) +
                (isMessageConstLength(thisGroup) ? "Length" : "ItemLengths.size()");

            final String minCheck = minCount > 0 ? countName + " < " + minCount + "LL || " : "";
            final String maxCheck = countName + " > " + maxCount + "LL";

            new Formatter(sbEncode).format("\n" +
                indent + "    length += %1$s::sbeHeaderSize();\n",
                formatClassName(groupToken.name()));

            if (isMessageConstLength(thisGroup))
            {
                new Formatter(sbEncode).format(
                    indent + "    if (%3$s%4$s)\n" +
                    indent + "    {\n" +
                    indent + "        throw std::runtime_error(\"%5$s outside of allowed range [E110]\");\n" +
                    indent + "    }\n" +
                    indent + "    length += %1$sLength *%2$s::sbeBlockLength();\n",
                    formatPropertyName(groupToken.name()),
                    formatClassName(groupToken.name()),
                    minCheck,
                    maxCheck,
                    countName);
            }
            else
            {
                new Formatter(sbEncode).format(
                    indent + "    if (%3$s%4$s)\n" +
                    indent + "    {\n" +
                    indent + "        throw std::runtime_error(\"%5$s outside of allowed range [E110]\");\n" +
                    indent + "    }\n\n" +
                    indent + "    for (const auto &e: %1$sItemLengths)\n" +
                    indent + "    {\n" +
                    indent + "        #if __cplusplus >= 201703L\n" +
                    indent + "        length += std::apply(%2$s::computeLength, e);\n" +
                    indent + "        #else\n" +
                    indent + "        length += %2$s::computeLength(%6$s);\n" +
                    indent + "        #endif\n" +
                    indent + "    }\n",
                    formatPropertyName(groupToken.name()),
                    formatClassName(groupToken.name()),
                    minCheck,
                    maxCheck,
                    countName,
                    generateMessageLengthCallPre17Helper(thisGroup));
            }

            new Formatter(sbSkip).format(
                indent + ("    auto &%1$sGroup { %1$s() };\n") +
                indent + ("    while (%1$sGroup.hasNext())\n") +
                indent + ("    {\n") +
                indent + ("        %1$sGroup.next().skip();\n") +
                indent + ("    }\n"),
                formatPropertyName(groupToken.name()));

            i = endSignal;
        }

        for (int i = 0, size = varData.size(); i < size;)
        {
            final Token varDataToken = varData.get(i);

            if (varDataToken.signal() != Signal.BEGIN_VAR_DATA)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_VAR_DATA: token=" + varDataToken);
            }

            final Token lengthToken = Generators.findFirst("length", varData, i);

            new Formatter(sbEncode).format("\n" +
                indent + "    length += %1$sHeaderLength();\n" +
                indent + "    if (%1$sLength > %2$dLL)\n" +
                indent + "    {\n" +
                indent + "        throw std::runtime_error(\"%1$sLength too long for length type [E109]\");\n" +
                indent + "    }\n" +
                indent + "    length += %1$sLength;\n",
                formatPropertyName(varDataToken.name()),
                lengthToken.encoding().applicableMaxValue().longValue());

            new Formatter(sbSkip).format(
                indent + "    skip%1$s();\n",
                toUpperFirstChar(varDataToken.name()));

            i += varDataToken.componentTokenCount();
        }

        final StringBuilder sb = new StringBuilder();

        new Formatter(sb).format("\n" +
            indent + "void skip()\n" +
            indent + "{\n" +
            sbSkip +
            indent + "}\n\n" +

            indent + "SBE_NODISCARD static SBE_CONSTEXPR bool isConstLength() SBE_NOEXCEPT\n" +
            indent + "{\n" +
            indent + "    return " + (groups.isEmpty() && varData.isEmpty()) + ";\n" +
            indent + "}\n\n" +

            indent + "SBE_NODISCARD static std::size_t computeLength(%1$s)\n" +
            indent + "{\n" +
            "#if defined(__GNUG__) && !defined(__clang__)\n" +
            "#pragma GCC diagnostic push\n" +
            "#pragma GCC diagnostic ignored \"-Wtype-limits\"\n" +
            "#endif\n" +
            indent + "    std::size_t length = sbeBlockLength();\n" +
            sbEncode + "\n" +
            indent + "    return length;\n" +
            "#if defined(__GNUG__) && !defined(__clang__)\n" +
            "#pragma GCC diagnostic pop\n" +
            "#endif\n" +
            indent + "}\n",
            generateMessageLengthArgs(groups, varData, indent + INDENT, true)[0]);

        return sb;
    }
}
