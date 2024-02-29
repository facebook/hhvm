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
package uk.co.real_logic.sbe.generation.java;

import uk.co.real_logic.sbe.PrimitiveType;
import uk.co.real_logic.sbe.generation.CodeGenerator;
import uk.co.real_logic.sbe.generation.Generators;
import uk.co.real_logic.sbe.generation.common.FieldPrecedenceModel;
import uk.co.real_logic.sbe.generation.common.PrecedenceChecks;
import uk.co.real_logic.sbe.ir.*;
import org.agrona.DirectBuffer;
import org.agrona.MutableDirectBuffer;
import org.agrona.Strings;
import org.agrona.Verify;
import org.agrona.collections.MutableBoolean;
import org.agrona.generation.DynamicPackageOutputManager;
import org.agrona.sbe.*;

import java.io.IOException;
import java.io.Writer;
import java.util.*;
import java.util.function.Function;

import static uk.co.real_logic.sbe.SbeTool.JAVA_INTERFACE_PACKAGE;
import static uk.co.real_logic.sbe.generation.java.JavaGenerator.CodecType.DECODER;
import static uk.co.real_logic.sbe.generation.java.JavaGenerator.CodecType.ENCODER;
import static uk.co.real_logic.sbe.generation.java.JavaUtil.*;
import static uk.co.real_logic.sbe.ir.GenerationUtil.*;

/**
 * Generate codecs for the Java 8 programming language.
 */
@SuppressWarnings("MethodLength")
public class JavaGenerator implements CodeGenerator
{
    static final String MESSAGE_HEADER_ENCODER_TYPE = "MessageHeaderEncoder";
    static final String MESSAGE_HEADER_DECODER_TYPE = "MessageHeaderDecoder";

    enum CodecType
    {
        DECODER,
        ENCODER
    }

    private static final String META_ATTRIBUTE_ENUM = "MetaAttribute";
    private static final String PACKAGE_INFO = "package-info";
    private static final String BASE_INDENT = "";
    private static final String INDENT = "    ";
    private static final Set<String> PACKAGES_EMPTY_SET = Collections.emptySet();

    private final Ir ir;
    private final DynamicPackageOutputManager outputManager;
    private final String fqMutableBuffer;
    private final String mutableBuffer;
    private final String fqReadOnlyBuffer;
    private final String readOnlyBuffer;
    private final boolean shouldGenerateGroupOrderAnnotation;
    private final boolean shouldGenerateInterfaces;
    private final boolean shouldDecodeUnknownEnumValues;
    private final boolean shouldSupportTypesPackageNames;
    private final PrecedenceChecks precedenceChecks;
    private final String precedenceChecksFlagName;
    private final String precedenceChecksPropName;
    private final Set<String> packageNameByTypes = new HashSet<>();

    /**
     * Create a new Java language {@link CodeGenerator}. Generator support for types in their own package is disabled.
     *
     * @param ir                                 for the messages and types.
     * @param mutableBuffer                      implementation used for mutating underlying buffers.
     * @param readOnlyBuffer                     implementation used for reading underlying buffers.
     * @param shouldGenerateGroupOrderAnnotation in the codecs.
     * @param shouldGenerateInterfaces           for common methods.
     * @param shouldDecodeUnknownEnumValues      generate support for unknown enum values when decoding.
     * @param outputManager                      for generating the codecs to.
     */
    public JavaGenerator(
        final Ir ir,
        final String mutableBuffer,
        final String readOnlyBuffer,
        final boolean shouldGenerateGroupOrderAnnotation,
        final boolean shouldGenerateInterfaces,
        final boolean shouldDecodeUnknownEnumValues,
        final DynamicPackageOutputManager outputManager)
    {
        this(ir, mutableBuffer, readOnlyBuffer, shouldGenerateGroupOrderAnnotation, shouldGenerateInterfaces,
            shouldDecodeUnknownEnumValues, false, outputManager);
    }

    /**
     * Create a new Java language {@link CodeGenerator}.
     *
     * @param ir                                 for the messages and types.
     * @param mutableBuffer                      implementation used for mutating underlying buffers.
     * @param readOnlyBuffer                     implementation used for reading underlying buffers.
     * @param shouldGenerateGroupOrderAnnotation in the codecs.
     * @param shouldGenerateInterfaces           for common methods.
     * @param shouldDecodeUnknownEnumValues      generate support for unknown enum values when decoding.
     * @param shouldSupportTypesPackageNames     generator support for types in their own package.
     * @param outputManager                      for generating the codecs to.
     */
    public JavaGenerator(
        final Ir ir,
        final String mutableBuffer,
        final String readOnlyBuffer,
        final boolean shouldGenerateGroupOrderAnnotation,
        final boolean shouldGenerateInterfaces,
        final boolean shouldDecodeUnknownEnumValues,
        final boolean shouldSupportTypesPackageNames,
        final DynamicPackageOutputManager outputManager)
    {
        this(
            ir,
            mutableBuffer,
            readOnlyBuffer,
            shouldGenerateGroupOrderAnnotation,
            shouldGenerateInterfaces,
            shouldDecodeUnknownEnumValues,
            shouldSupportTypesPackageNames,
            PrecedenceChecks.newInstance(new PrecedenceChecks.Context()),
            outputManager);
    }

    /**
     * Create a new Java language {@link CodeGenerator}.
     *
     * @param ir                                 for the messages and types.
     * @param mutableBuffer                      implementation used for mutating underlying buffers.
     * @param readOnlyBuffer                     implementation used for reading underlying buffers.
     * @param shouldGenerateGroupOrderAnnotation in the codecs.
     * @param shouldGenerateInterfaces           for common methods.
     * @param shouldDecodeUnknownEnumValues      generate support for unknown enum values when decoding.
     * @param shouldSupportTypesPackageNames     generator support for types in their own package.
     * @param precedenceChecks                   whether and how to generate field precedence checks.
     * @param outputManager                      for generating the codecs to.
     */
    public JavaGenerator(
        final Ir ir,
        final String mutableBuffer,
        final String readOnlyBuffer,
        final boolean shouldGenerateGroupOrderAnnotation,
        final boolean shouldGenerateInterfaces,
        final boolean shouldDecodeUnknownEnumValues,
        final boolean shouldSupportTypesPackageNames,
        final PrecedenceChecks precedenceChecks,
        final DynamicPackageOutputManager outputManager)
    {
        Verify.notNull(ir, "ir");
        Verify.notNull(outputManager, "outputManager");

        this.ir = ir;
        this.shouldSupportTypesPackageNames = shouldSupportTypesPackageNames;
        this.outputManager = outputManager;

        this.mutableBuffer = validateBufferImplementation(mutableBuffer, MutableDirectBuffer.class);
        this.fqMutableBuffer = mutableBuffer;

        this.readOnlyBuffer = validateBufferImplementation(readOnlyBuffer, DirectBuffer.class);
        this.fqReadOnlyBuffer = readOnlyBuffer;

        this.shouldGenerateGroupOrderAnnotation = shouldGenerateGroupOrderAnnotation;
        this.shouldGenerateInterfaces = shouldGenerateInterfaces;
        this.shouldDecodeUnknownEnumValues = shouldDecodeUnknownEnumValues;

        this.precedenceChecks = precedenceChecks;
        this.precedenceChecksFlagName = precedenceChecks.context().precedenceChecksFlagName();
        this.precedenceChecksPropName = precedenceChecks.context().precedenceChecksPropName();
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
     * Register the types explicit package - if set and should be supported.
     *
     * @param token the 0-th token of the type.
     * @param ir    the intermediate representation.
     * @return the overridden package name of the type if set and supported, or {@link Ir#applicableNamespace()}.
     */
    private String registerTypesPackageName(final Token token, final Ir ir)
    {
        if (!shouldSupportTypesPackageNames)
        {
            return ir.applicableNamespace();
        }

        if (token.packageName() != null)
        {
            packageNameByTypes.add(token.packageName());
            outputManager.setPackageName(token.packageName());
            return token.packageName();
        }

        return ir.applicableNamespace();
    }

    /**
     * {@inheritDoc}
     */
    public void generate() throws IOException
    {
        packageNameByTypes.clear();
        generatePackageInfo();
        generateTypeStubs();
        generateMessageHeaderStub();

        for (final List<Token> tokens : ir.messages())
        {
            final Token msgToken = tokens.get(0);
            final List<Token> messageBody = getMessageBody(tokens);
            final boolean hasVarData = -1 != findSignal(messageBody, Signal.BEGIN_VAR_DATA);

            int i = 0;
            final List<Token> fields = new ArrayList<>();
            i = collectFields(messageBody, i, fields);

            final List<Token> groups = new ArrayList<>();
            i = collectGroups(messageBody, i, groups);

            final List<Token> varData = new ArrayList<>();
            collectVarData(messageBody, i, varData);

            final String decoderClassName = formatClassName(decoderName(msgToken.name()));
            final String decoderStateClassName = decoderClassName + "#CodecStates";
            final FieldPrecedenceModel decoderPrecedenceModel =
                precedenceChecks.createDecoderModel(decoderStateClassName, tokens);
            generateDecoder(decoderClassName, msgToken, fields, groups, varData, hasVarData, decoderPrecedenceModel);

            final String encoderClassName = formatClassName(encoderName(msgToken.name()));
            final String encoderStateClassName = encoderClassName + "#CodecStates";
            final FieldPrecedenceModel encoderPrecedenceModel =
                precedenceChecks.createEncoderModel(encoderStateClassName, tokens);
            generateEncoder(encoderClassName, msgToken, fields, groups, varData, hasVarData, encoderPrecedenceModel);
        }
    }

    private void generateEncoder(
        final String className,
        final Token msgToken,
        final List<Token> fields,
        final List<Token> groups,
        final List<Token> varData,
        final boolean hasVarData,
        final FieldPrecedenceModel fieldPrecedenceModel)
        throws IOException
    {
        final String implementsString = implementsInterface(MessageEncoderFlyweight.class.getSimpleName());

        try (Writer out = outputManager.createOutput(className))
        {
            out.append(generateMainHeader(ir.applicableNamespace(), ENCODER, hasVarData));

            if (shouldGenerateGroupOrderAnnotation)
            {
                generateAnnotations(BASE_INDENT, className, groups, out, this::encoderName);
            }
            out.append(generateDeclaration(className, implementsString, msgToken));

            out.append(generateFieldOrderStates(fieldPrecedenceModel));
            out.append(generateEncoderFlyweightCode(className, fieldPrecedenceModel, msgToken));

            final StringBuilder sb = new StringBuilder();
            generateEncoderFields(sb, className, fieldPrecedenceModel, fields, BASE_INDENT);
            generateEncoderGroups(sb, className, fieldPrecedenceModel, groups, BASE_INDENT, false);
            generateEncoderVarData(sb, className, fieldPrecedenceModel, varData, BASE_INDENT);

            generateEncoderDisplay(sb, decoderName(msgToken.name()));
            generateFullyEncodedCheck(sb, fieldPrecedenceModel);

            out.append(sb);
            out.append("}\n");
        }
    }

    private static CharSequence qualifiedStateCase(final FieldPrecedenceModel.State state)
    {
        return "CodecStates." + state.name();
    }

    private static CharSequence stateCaseForSwitchCase(final FieldPrecedenceModel.State state)
    {
        return qualifiedStateCase(state);
    }

    private static CharSequence unqualifiedStateCase(final FieldPrecedenceModel.State state)
    {
        return state.name();
    }

    private CharSequence generateFieldOrderStates(final FieldPrecedenceModel fieldPrecedenceModel)
    {
        if (null == fieldPrecedenceModel)
        {
            return "";
        }

        final StringBuilder sb = new StringBuilder();

        sb.append("    private static final boolean ENABLE_BOUNDS_CHECKS = ")
            .append("!Boolean.getBoolean(\"agrona.disable.bounds.checks\");\n\n");
        sb.append("    private static final boolean ")
            .append(precedenceChecksFlagName).append(" = ")
            .append("Boolean.parseBoolean(System.getProperty(\n")
            .append("        \"").append(precedenceChecksPropName).append("\",\n")
            .append("        Boolean.toString(ENABLE_BOUNDS_CHECKS)));\n\n");

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
        sb.append("    private static class CodecStates\n")
            .append("    {\n");
        fieldPrecedenceModel.forEachStateOrderedByStateNumber((state) ->
            sb.append("        private static final int ")
            .append(unqualifiedStateCase(state))
            .append(" = ").append(state.number())
            .append(";\n"));

        sb.append("\n").append("        private static final String[] STATE_NAME_LOOKUP =\n")
                .append("        {\n");
        fieldPrecedenceModel.forEachStateOrderedByStateNumber((state) ->
            sb.append("            \"").append(state.name()).append("\",\n"));
        sb.append("        };\n\n");

        sb.append("        private static final String[] STATE_TRANSITIONS_LOOKUP =\n")
                .append("        {\n");
        fieldPrecedenceModel.forEachStateOrderedByStateNumber((state) ->
        {
            sb.append("            \"");
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
        sb.append("        };\n\n");

        sb.append("        private static String name(final int state)\n")
            .append("        {\n")
            .append("            return STATE_NAME_LOOKUP[state];\n")
            .append("        }\n\n");

        sb.append("        private static String transitions(final int state)\n")
            .append("        {\n")
            .append("            return STATE_TRANSITIONS_LOOKUP[state];\n")
            .append("        }\n");

        sb.append("    }\n\n");

        sb.append("    private int codecState = ")
            .append(qualifiedStateCase(fieldPrecedenceModel.notWrappedState()))
            .append(";\n\n");

        sb.append("    private int codecState()\n")
            .append("    {\n")
            .append("        return codecState;\n")
            .append("    }\n\n");

        sb.append("    private void codecState(int newState)\n")
            .append("    {\n")
            .append("        codecState = newState;\n")
            .append("    }\n\n");

        return sb;
    }

    private void generateFullyEncodedCheck(
        final StringBuilder sb,
        final FieldPrecedenceModel fieldPrecedenceModel)
    {
        if (null == fieldPrecedenceModel)
        {
            return;
        }

        sb.append("\n");

        sb.append("    public void checkEncodingIsComplete()\n")
            .append("    {\n")
            .append("        if (").append(precedenceChecksFlagName).append(")\n")
            .append("        {\n")
            .append("            switch (codecState)\n")
            .append("            {\n");

        fieldPrecedenceModel.forEachTerminalEncoderState((state) ->
            sb.append("                case ").append(stateCaseForSwitchCase(state)).append(":\n")
            .append("                    return;\n"));

        sb.append("                default:\n")
            .append("                    throw new IllegalStateException(\"Not fully encoded, current state: \" +\n")
            .append("                        CodecStates.name(codecState) + \", allowed transitions: \" +\n")
            .append("                        CodecStates.transitions(codecState));\n")
            .append("            }\n")
            .append("        }\n")
            .append("    }\n\n");
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

        sb.append("\n")
            .append(indent).append("private void ").append(accessOrderListenerMethodName(token)).append("()\n")
            .append(indent).append("{\n");

        final FieldPrecedenceModel.CodecInteraction fieldAccess =
            fieldPrecedenceModel.interactionFactory().accessField(token);

        generateAccessOrderListener(
            sb,
            indent + "    ",
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
        sb.append(indent).append("if (").append(precedenceChecksFlagName).append(")\n")
            .append(indent).append("{\n")
            .append(indent).append("    ").append(methodName).append("(");

        for (int i = 0; i < arguments.length; i++)
        {
            if (i > 0)
            {
                sb.append(", ");
            }
            sb.append(arguments[i]);
        }
        sb.append(");\n");

        sb.append(indent).append("}\n\n");

        return sb;
    }

    private static void generateAccessOrderListenerMethodForGroupWrap(
        final StringBuilder sb,
        final String action,
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
            .append("(final int count)\n")
            .append(indent).append("{\n")
            .append(indent).append("    if (count == 0)\n")
            .append(indent).append("    {\n");

        final FieldPrecedenceModel.CodecInteraction selectEmptyGroup =
            fieldPrecedenceModel.interactionFactory().determineGroupIsEmpty(token);

        generateAccessOrderListener(
            sb,
            indent + "        ",
            action + " count of repeating group",
            fieldPrecedenceModel,
            selectEmptyGroup);

        sb.append(indent).append("    }\n")
            .append(indent).append("    else\n")
            .append(indent).append("    {\n");

        final FieldPrecedenceModel.CodecInteraction selectNonEmptyGroup =
            fieldPrecedenceModel.interactionFactory().determineGroupHasElements(token);

        generateAccessOrderListener(
            sb,
            indent + "        ",
            action + " count of repeating group",
            fieldPrecedenceModel,
            selectNonEmptyGroup);

        sb.append(indent).append("    }\n")
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
            generateAccessOrderException(sb, indent + "    ", action, fieldPrecedenceModel, interaction);
            sb.append(indent).append("}\n");
        }
        else
        {
            sb.append(indent).append("switch (codecState())\n")
                .append(indent).append("{\n");

            fieldPrecedenceModel.forEachTransition(interaction, (transitionGroup) ->
            {
                transitionGroup.forEachStartState((startState) ->
                    sb.append(indent).append("    case ").append(stateCaseForSwitchCase(startState)).append(":\n"));
                sb.append(indent).append("        codecState(")
                    .append(qualifiedStateCase(transitionGroup.endState())).append(");\n")
                    .append(indent).append("        break;\n");
            });

            sb.append(indent).append("    default:\n");
            generateAccessOrderException(sb, indent + "        ", action, fieldPrecedenceModel, interaction);
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
        sb.append(indent).append("throw new IllegalStateException(")
            .append("\"Illegal field access order. \" +\n")
            .append(indent)
            .append("    \"Cannot ").append(action).append(" \\\"").append(interaction.groupQualifiedName())
            .append("\\\" in state: \" + CodecStates.name(codecState()) +\n")
            .append(indent)
            .append("    \". Expected one of these transitions: [\" + CodecStates.transitions(codecState()) +\n")
            .append(indent)
            .append("    \"]. Please see the diagram in the Javadoc of the class ")
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

        sb.append(indent).append("private void onNextElementAccessed()\n")
            .append(indent).append("{\n")
            .append(indent).append("    final int remaining = ").append("count - index").append(";\n")
            .append(indent).append("    if (remaining > 1)\n")
            .append(indent).append("    {\n");

        final FieldPrecedenceModel.CodecInteraction selectNextElementInGroup =
            fieldPrecedenceModel.interactionFactory().moveToNextElement(token);

        generateAccessOrderListener(
            sb,
            indent + "       ",
            "access next element in repeating group",
            fieldPrecedenceModel,
            selectNextElementInGroup);

        sb.append(indent).append("    }\n")
            .append(indent).append("    else if (remaining == 1)\n")
            .append(indent).append("    {\n");

        final FieldPrecedenceModel.CodecInteraction selectLastElementInGroup =
            fieldPrecedenceModel.interactionFactory().moveToLastElement(token);

        generateAccessOrderListener(
            sb,
            indent + "        ",
            "access next element in repeating group",
            fieldPrecedenceModel,
            selectLastElementInGroup);

        sb.append(indent).append("    }\n")
            .append(indent).append("}\n\n");
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

        sb.append(indent).append("private void onResetCountToIndex()\n")
            .append(indent).append("{\n");

        final FieldPrecedenceModel.CodecInteraction resetCountToIndex =
            fieldPrecedenceModel.interactionFactory().resetCountToIndex(token);

        generateAccessOrderListener(
            sb,
            indent + "   ",
            "reset count of repeating group",
            fieldPrecedenceModel,
            resetCountToIndex);

        sb.append(indent).append("}\n\n");
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
            .append(indent).append("void ").append(accessOrderListenerMethodName(token, "Length")).append("()\n")
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
        sb.append(indent).append("private void onWrap(final int actingVersion)\n")
            .append(indent).append("{\n")
            .append(indent).append("    switch(actingVersion)\n")
            .append(indent).append("    {\n");

        fieldPrecedenceModel.forEachWrappedStateByVersion((version, state) ->
            sb.append(indent).append("        case ").append(version).append(":\n")
            .append(indent).append("            codecState(")
            .append(qualifiedStateCase(state)).append(");\n")
            .append(indent).append("            break;\n"));

        sb.append(indent).append("        default:\n")
            .append(indent).append("            codecState(")
            .append(qualifiedStateCase(fieldPrecedenceModel.latestVersionWrappedState())).append(");\n")
            .append(indent).append("            break;\n")
            .append(indent).append("    }\n")
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
        sb.append(indent).append("if (").append(precedenceChecksFlagName).append(")")
            .append("\n").append(indent).append("{\n")
            .append(indent).append("    codecState(")
            .append(qualifiedStateCase(fieldPrecedenceModel.latestVersionWrappedState()))
            .append(");\n")
            .append(indent).append("}\n\n");
        return sb;
    }

    private void generateDecoder(
        final String className,
        final Token msgToken,
        final List<Token> fields,
        final List<Token> groups,
        final List<Token> varData,
        final boolean hasVarData,
        final FieldPrecedenceModel fieldPrecedenceModel)
        throws IOException
    {
        final String implementsString = implementsInterface(MessageDecoderFlyweight.class.getSimpleName());

        try (Writer out = outputManager.createOutput(className))
        {
            out.append(generateMainHeader(ir.applicableNamespace(), DECODER, hasVarData));

            if (shouldGenerateGroupOrderAnnotation)
            {
                generateAnnotations(BASE_INDENT, className, groups, out, this::decoderName);
            }

            out.append(generateDeclaration(className, implementsString, msgToken));
            out.append(generateFieldOrderStates(fieldPrecedenceModel));
            out.append(generateDecoderFlyweightCode(fieldPrecedenceModel, className, msgToken));

            final StringBuilder sb = new StringBuilder();
            generateDecoderFields(sb, fieldPrecedenceModel, fields, BASE_INDENT);
            generateDecoderGroups(sb, fieldPrecedenceModel, className, groups, BASE_INDENT, false);
            generateDecoderVarData(sb, fieldPrecedenceModel, varData, BASE_INDENT);

            generateDecoderDisplay(sb, msgToken.name(), fields, groups, varData);
            generateMessageLength(sb, className, true, groups, varData, BASE_INDENT);

            out.append(sb);
            out.append("}\n");
        }
    }

    private void generateDecoderGroups(
        final StringBuilder sb,
        final FieldPrecedenceModel fieldPrecedenceModel, final String outerClassName,
        final List<Token> tokens,
        final String indent,
        final boolean isSubGroup) throws IOException
    {
        for (int i = 0, size = tokens.size(); i < size; i++)
        {
            final Token groupToken = tokens.get(i);
            if (groupToken.signal() != Signal.BEGIN_GROUP)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_GROUP: token=" + groupToken);
            }

            final int index = i;
            final String groupName = decoderName(formatClassName(groupToken.name()));

            ++i;
            final int groupHeaderTokenCount = tokens.get(i).componentTokenCount();
            i += groupHeaderTokenCount;

            final List<Token> fields = new ArrayList<>();
            i = collectFields(tokens, i, fields);

            final List<Token> groups = new ArrayList<>();
            i = collectGroups(tokens, i, groups);

            final List<Token> varData = new ArrayList<>();
            i = collectVarData(tokens, i, varData);

            generateGroupDecoderProperty(sb, groupName, fieldPrecedenceModel, groupToken, indent, isSubGroup);
            generateTypeJavadoc(sb, indent + INDENT, groupToken);

            if (shouldGenerateGroupOrderAnnotation)
            {
                generateAnnotations(indent + INDENT, groupName, groups, sb, this::decoderName);
            }
            generateGroupDecoderClassHeader(sb, groupName, outerClassName, fieldPrecedenceModel, groupToken,
                tokens, groups, index, indent + INDENT);

            generateDecoderFields(sb, fieldPrecedenceModel, fields, indent + INDENT);
            generateDecoderGroups(sb, fieldPrecedenceModel, outerClassName, groups, indent + INDENT, true);
            generateDecoderVarData(sb, fieldPrecedenceModel, varData, indent + INDENT);

            appendGroupInstanceDecoderDisplay(sb, fields, groups, varData, indent + INDENT);
            generateMessageLength(sb, groupName, false, groups, varData, indent + INDENT);

            sb.append(indent).append("    }\n");
        }
    }

    private void generateEncoderGroups(
        final StringBuilder sb,
        final String outerClassName,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final List<Token> tokens,
        final String indent,
        final boolean isSubGroup) throws IOException
    {
        for (int i = 0, size = tokens.size(); i < size; i++)
        {
            final Token groupToken = tokens.get(i);
            if (groupToken.signal() != Signal.BEGIN_GROUP)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_GROUP: token=" + groupToken);
            }

            final int index = i;
            final String groupName = groupToken.name();
            final String groupClassName = encoderName(groupName);

            ++i;
            final int groupHeaderTokenCount = tokens.get(i).componentTokenCount();
            i += groupHeaderTokenCount;

            final List<Token> fields = new ArrayList<>();
            i = collectFields(tokens, i, fields);

            final List<Token> groups = new ArrayList<>();
            i = collectGroups(tokens, i, groups);

            final List<Token> varData = new ArrayList<>();
            i = collectVarData(tokens, i, varData);

            generateGroupEncoderProperty(sb, groupName, fieldPrecedenceModel, groupToken, indent, isSubGroup);
            generateTypeJavadoc(sb, indent + INDENT, groupToken);

            if (shouldGenerateGroupOrderAnnotation)
            {
                generateAnnotations(indent + INDENT, groupClassName, groups, sb, this::encoderName);
            }
            generateGroupEncoderClassHeader(
                sb, groupName, outerClassName, fieldPrecedenceModel, groupToken,
                tokens, groups, index, indent + INDENT);

            generateEncoderFields(sb, groupClassName, fieldPrecedenceModel, fields, indent + INDENT);
            generateEncoderGroups(sb, outerClassName, fieldPrecedenceModel, groups, indent + INDENT, true);
            generateEncoderVarData(sb, groupClassName, fieldPrecedenceModel, varData, indent + INDENT);

            sb.append(indent).append("    }\n");
        }
    }

    private void generateGroupDecoderClassHeader(
        final StringBuilder sb,
        final String groupName,
        final String parentMessageClassName,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final Token groupToken,
        final List<Token> tokens,
        final List<Token> subGroupTokens,
        final int index,
        final String indent)
    {
        final String className = formatClassName(groupName);
        final int dimensionHeaderLen = tokens.get(index + 1).encodedLength();

        final Token blockLengthToken = Generators.findFirst("blockLength", tokens, index);
        final Token numInGroupToken = Generators.findFirst("numInGroup", tokens, index);

        final PrimitiveType blockLengthType = blockLengthToken.encoding().primitiveType();
        final String blockLengthOffset = "limit + " + blockLengthToken.offset();
        final String blockLengthGet = generateGet(
            blockLengthType, blockLengthOffset, byteOrderString(blockLengthToken.encoding()));

        final PrimitiveType numInGroupType = numInGroupToken.encoding().primitiveType();
        final String numInGroupOffset = "limit + " + numInGroupToken.offset();
        final String numInGroupGet = generateGet(
            numInGroupType, numInGroupOffset, byteOrderString(numInGroupToken.encoding()));

        generateGroupDecoderClassDeclaration(
            sb,
            groupName,
            parentMessageClassName,
            findSubGroupNames(subGroupTokens),
            indent,
            dimensionHeaderLen);

        final String blockLenCast = PrimitiveType.UINT32 == blockLengthType ? "(int)" : "";
        final String numInGroupCast = PrimitiveType.UINT32 == numInGroupType ? "(int)" : "";

        sb.append("\n")
            .append(indent).append("    public void wrap(final ").append(readOnlyBuffer).append(" buffer)\n")
            .append(indent).append("    {\n")
            .append(indent).append("        if (buffer != this.buffer)\n")
            .append(indent).append("        {\n")
            .append(indent).append("            this.buffer = buffer;\n")
            .append(indent).append("        }\n\n")
            .append(indent).append("        index = 0;\n")
            .append(indent).append("        final int limit = parentMessage.limit();\n")
            .append(indent).append("        parentMessage.limit(limit + HEADER_SIZE);\n")
            .append(indent).append("        blockLength = ").append(blockLenCast).append(blockLengthGet).append(";\n")
            .append(indent).append("        count = ").append(numInGroupCast).append(numInGroupGet).append(";\n")
            .append(indent).append("    }\n\n");


        generateAccessOrderListenerMethodForNextGroupElement(sb, fieldPrecedenceModel, indent + "    ", groupToken);

        sb.append(indent).append("    public ").append(className).append(" next()\n")
            .append(indent).append("    {\n")
            .append(indent).append("        if (index >= count)\n")
            .append(indent).append("        {\n")
            .append(indent).append("            throw new java.util.NoSuchElementException();\n")
            .append(indent).append("        }\n\n")
            .append(generateAccessOrderListenerCall(fieldPrecedenceModel, indent + "        ", "onNextElementAccessed"))
            .append(indent).append("        offset = parentMessage.limit();\n")
            .append(indent).append("        parentMessage.limit(offset + blockLength);\n")
            .append(indent).append("        ++index;\n\n")
            .append(indent).append("        return this;\n")
            .append(indent).append("    }\n");

        final String numInGroupJavaTypeName = javaTypeName(numInGroupType);
        final String numInGroupMinValue = generateLiteral(
            numInGroupType, numInGroupToken.encoding().applicableMinValue().toString());
        generatePrimitiveFieldMetaMethod(sb, indent, numInGroupJavaTypeName, "count", "Min", numInGroupMinValue);
        final String numInGroupMaxValue = generateLiteral(
            numInGroupType, numInGroupToken.encoding().applicableMaxValue().toString());
        generatePrimitiveFieldMetaMethod(sb, indent, numInGroupJavaTypeName, "count", "Max", numInGroupMaxValue);

        sb.append("\n")
            .append(indent).append("    public static int sbeHeaderSize()\n")
            .append(indent).append("    {\n")
            .append(indent).append("        return HEADER_SIZE;\n")
            .append(indent).append("    }\n");

        sb.append("\n")
            .append(indent).append("    public static int sbeBlockLength()\n")
            .append(indent).append("    {\n")
            .append(indent).append("        return ").append(tokens.get(index).encodedLength()).append(";\n")
            .append(indent).append("    }\n");

        sb.append("\n")
            .append(indent).append("    public int actingBlockLength()\n")
            .append(indent).append("    {\n")
            .append(indent).append("        return blockLength;\n")
            .append(indent).append("    }\n\n")
            .append(indent).append("    public int count()\n")
            .append(indent).append("    {\n")
            .append(indent).append("        return count;\n")
            .append(indent).append("    }\n\n")
            .append(indent).append("    public java.util.Iterator<").append(className).append("> iterator()\n")
            .append(indent).append("    {\n")
            .append(indent).append("        return this;\n")
            .append(indent).append("    }\n\n")
            .append(indent).append("    public void remove()\n")
            .append(indent).append("    {\n")
            .append(indent).append("        throw new UnsupportedOperationException();\n")
            .append(indent).append("    }\n\n")
            .append(indent).append("    public boolean hasNext()\n")
            .append(indent).append("    {\n")
            .append(indent).append("        return index < count;\n")
            .append(indent).append("    }\n");

        if (null != fieldPrecedenceModel)
        {
            sb.append("\n")
                .append(indent).append("    private int codecState()\n")
                .append(indent).append("    {\n")
                .append(indent).append("        return parentMessage.codecState();\n")
                .append(indent).append("    }\n");

            sb.append("\n")
                .append(indent).append("    private void codecState(final int newState)\n")
                .append(indent).append("    {\n")
                .append(indent).append("        parentMessage.codecState(newState);\n")
                .append(indent).append("    }\n");
        }
    }

    private void generateGroupEncoderClassHeader(
        final StringBuilder sb,
        final String groupName,
        final String parentMessageClassName,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final Token groupToken,
        final List<Token> tokens,
        final List<Token> subGroupTokens,
        final int index,
        final String ind)
    {
        final int dimensionHeaderSize = tokens.get(index + 1).encodedLength();

        generateGroupEncoderClassDeclaration(
            sb,
            groupName,
            parentMessageClassName,
            findSubGroupNames(subGroupTokens),
            ind,
            dimensionHeaderSize);

        final int blockLength = tokens.get(index).encodedLength();
        final Token blockLengthToken = Generators.findFirst("blockLength", tokens, index);
        final Token numInGroupToken = Generators.findFirst("numInGroup", tokens, index);

        final PrimitiveType blockLengthType = blockLengthToken.encoding().primitiveType();
        final String blockLengthOffset = "limit + " + blockLengthToken.offset();
        final String blockLengthValue = Integer.toString(blockLength);
        final String blockLengthPut = generatePut(
            blockLengthType, blockLengthOffset, blockLengthValue, byteOrderString(blockLengthToken.encoding()));

        final PrimitiveType numInGroupType = numInGroupToken.encoding().primitiveType();

        final PrimitiveType numInGroupTypeCast = PrimitiveType.UINT32 == numInGroupType ?
            PrimitiveType.INT32 : numInGroupType;
        final String numInGroupOffset = "limit + " + numInGroupToken.offset();
        final String numInGroupValue = "count";
        final String numInGroupPut = generatePut(
            numInGroupTypeCast, numInGroupOffset, numInGroupValue, byteOrderString(numInGroupToken.encoding()));

        new Formatter(sb).format("\n" +
            ind + "    public void wrap(final %2$s buffer, final int count)\n" +
            ind + "    {\n" +
            ind + "        if (count < %3$d || count > %4$d)\n" +
            ind + "        {\n" +
            ind + "            throw new IllegalArgumentException(\"count outside allowed range: count=\" + count);\n" +
            ind + "        }\n\n" +
            ind + "        if (buffer != this.buffer)\n" +
            ind + "        {\n" +
            ind + "            this.buffer = buffer;\n" +
            ind + "        }\n\n" +
            ind + "        index = 0;\n" +
            ind + "        this.count = count;\n" +
            ind + "        final int limit = parentMessage.limit();\n" +
            ind + "        initialLimit = limit;\n" +
            ind + "        parentMessage.limit(limit + HEADER_SIZE);\n" +
            ind + "        %5$s;\n" +
            ind + "        %6$s;\n" +
            ind + "    }\n\n",
            parentMessageClassName,
            mutableBuffer,
            numInGroupToken.encoding().applicableMinValue().longValue(),
            numInGroupToken.encoding().applicableMaxValue().longValue(),
            blockLengthPut,
            numInGroupPut);

        generateAccessOrderListenerMethodForNextGroupElement(sb, fieldPrecedenceModel, ind + "    ", groupToken);
        generateAccessOrderListenerMethodForResetGroupCount(sb, fieldPrecedenceModel, ind + "    ", groupToken);

        sb.append(ind).append("    public ").append(encoderName(groupName)).append(" next()\n")
            .append(ind).append("    {\n")
            .append(generateAccessOrderListenerCall(fieldPrecedenceModel, ind + "        ", "onNextElementAccessed"))
            .append(ind).append("        if (index >= count)\n")
            .append(ind).append("        {\n")
            .append(ind).append("            throw new java.util.NoSuchElementException();\n")
            .append(ind).append("        }\n\n")
            .append(ind).append("        offset = parentMessage.limit();\n")
            .append(ind).append("        parentMessage.limit(offset + sbeBlockLength());\n")
            .append(ind).append("        ++index;\n\n")
            .append(ind).append("        return this;\n")
            .append(ind).append("    }\n\n");

        final String countOffset = "initialLimit + " + numInGroupToken.offset();
        final String resetCountPut = generatePut(
            numInGroupTypeCast, countOffset, numInGroupValue, byteOrderString(numInGroupToken.encoding()));

        sb.append(ind).append("    public int resetCountToIndex()\n")
            .append(ind).append("    {\n")
            .append(generateAccessOrderListenerCall(fieldPrecedenceModel, ind + "        ", "onResetCountToIndex"))
            .append(ind).append("        count = index;\n")
            .append(ind).append("        ").append(resetCountPut).append(";\n\n")
            .append(ind).append("        return count;\n")
            .append(ind).append("    }\n");

        final String numInGroupJavaTypeName = javaTypeName(numInGroupType);
        final String numInGroupMinValue = generateLiteral(
            numInGroupType, numInGroupToken.encoding().applicableMinValue().toString());
        generatePrimitiveFieldMetaMethod(sb, ind, numInGroupJavaTypeName, "count", "Min", numInGroupMinValue);
        final String numInGroupMaxValue = generateLiteral(
            numInGroupType, numInGroupToken.encoding().applicableMaxValue().toString());
        generatePrimitiveFieldMetaMethod(sb, ind, numInGroupJavaTypeName, "count", "Max", numInGroupMaxValue);

        sb.append("\n")
            .append(ind).append("    public static int sbeHeaderSize()\n")
            .append(ind).append("    {\n")
            .append(ind).append("        return HEADER_SIZE;\n")
            .append(ind).append("    }\n");

        sb.append("\n")
            .append(ind).append("    public static int sbeBlockLength()\n")
            .append(ind).append("    {\n")
            .append(ind).append("        return ").append(blockLength).append(";\n")
            .append(ind).append("    }\n");

        if (null != fieldPrecedenceModel)
        {
            sb.append("\n")
                .append(ind).append("    private int codecState()\n")
                .append(ind).append("    {\n")
                .append(ind).append("        return parentMessage.codecState();\n")
                .append(ind).append("    }\n");

            sb.append("\n")
                .append(ind).append("    private void codecState(final int newState)\n")
                .append(ind).append("    {\n")
                .append(ind).append("        parentMessage.codecState(newState);\n")
                .append(ind).append("    }\n");
        }
    }

    private static String primitiveTypeName(final Token token)
    {
        return javaTypeName(token.encoding().primitiveType());
    }

    private void generateGroupDecoderClassDeclaration(
        final StringBuilder sb,
        final String groupName,
        final String parentMessageClassName,
        final List<String> subGroupNames,
        final String indent,
        final int dimensionHeaderSize)
    {
        final String className = formatClassName(groupName);

        new Formatter(sb).format("\n" +
            indent + "public static final class %1$s\n" +
            indent + "    implements Iterable<%1$s>, java.util.Iterator<%1$s>\n" +
            indent + "{\n" +
            indent + "    public static final int HEADER_SIZE = %2$d;\n" +
            indent + "    private final %3$s parentMessage;\n" +
            indent + "    private %4$s buffer;\n" +
            indent + "    private int count;\n" +
            indent + "    private int index;\n" +
            indent + "    private int offset;\n" +
            indent + "    private int blockLength;\n",
            className,
            dimensionHeaderSize,
            parentMessageClassName,
            readOnlyBuffer);

        for (final String subGroupName : subGroupNames)
        {
            final String type = formatClassName(decoderName(subGroupName));
            final String field = formatPropertyName(subGroupName);
            sb.append(indent).append("    private final ").append(type).append(" ").append(field).append(";\n");
        }

        sb
            .append("\n")
            .append(indent).append("    ")
            .append(className).append("(final ").append(parentMessageClassName).append(" parentMessage)\n")
            .append(indent).append("    {\n")
            .append(indent).append("        this.parentMessage = parentMessage;\n");

        for (final String subGroupName : subGroupNames)
        {
            final String type = formatClassName(decoderName(subGroupName));
            final String field = formatPropertyName(subGroupName);
            sb
                .append(indent).append("        ")
                .append(field).append(" = new ").append(type).append("(parentMessage);\n");
        }

        sb.append(indent).append("    }\n");
    }

    private void generateGroupEncoderClassDeclaration(
        final StringBuilder sb,
        final String groupName,
        final String parentMessageClassName,
        final List<String> subGroupNames,
        final String indent,
        final int dimensionHeaderSize)
    {
        final String className = encoderName(groupName);

        new Formatter(sb).format("\n" +
            indent + "public static final class %1$s\n" +
            indent + "{\n" +
            indent + "    public static final int HEADER_SIZE = %2$d;\n" +
            indent + "    private final %3$s parentMessage;\n" +
            indent + "    private %4$s buffer;\n" +
            indent + "    private int count;\n" +
            indent + "    private int index;\n" +
            indent + "    private int offset;\n" +
            indent + "    private int initialLimit;\n",
            className,
            dimensionHeaderSize,
            parentMessageClassName,
            mutableBuffer);

        for (final String subGroupName : subGroupNames)
        {
            final String type = encoderName(subGroupName);
            final String field = formatPropertyName(subGroupName);
            sb.append(indent).append("    private final ").append(type).append(" ").append(field).append(";\n");
        }

        sb
            .append("\n")
            .append(indent).append("    ")
            .append(className).append("(final ").append(parentMessageClassName).append(" parentMessage)\n")
            .append(indent).append("    {\n")
            .append(indent).append("        this.parentMessage = parentMessage;\n");

        for (final String subGroupName : subGroupNames)
        {
            final String type = encoderName(subGroupName);
            final String field = formatPropertyName(subGroupName);
            sb
                .append(indent).append("        ")
                .append(field).append(" = new ").append(type).append("(parentMessage);\n");
        }

        sb.append(indent).append("    }\n");
    }

    private void generateGroupDecoderProperty(
        final StringBuilder sb,
        final String groupName,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final Token token,
        final String indent,
        final boolean isSubGroup)
    {
        final String className = formatClassName(groupName);
        final String propertyName = formatPropertyName(token.name());

        if (!isSubGroup)
        {
            new Formatter(sb).format("\n" +
                indent + "    private final %s %s = new %s(this);\n",
                className,
                propertyName,
                className);
        }

        new Formatter(sb).format("\n" +
            indent + "    public static long %sId()\n" +
            indent + "    {\n" +
            indent + "        return %d;\n" +
            indent + "    }\n",
            formatPropertyName(groupName),
            token.id());

        new Formatter(sb).format("\n" +
            indent + "    public static int %sSinceVersion()\n" +
            indent + "    {\n" +
            indent + "        return %d;\n" +
            indent + "    }\n",
            formatPropertyName(groupName),
            token.version());

        final String actingVersionGuard = token.version() == 0 ?
            "" :
            indent + "        if (parentMessage.actingVersion < " + token.version() + ")\n" +
            indent + "        {\n" +
            indent + "            " + propertyName + ".count = 0;\n" +
            indent + "            " + propertyName + ".index = 0;\n" +
            indent + "            return " + propertyName + ";\n" +
            indent + "        }\n\n";

        generateAccessOrderListenerMethodForGroupWrap(sb, "decode", fieldPrecedenceModel, indent + "    ", token);

        generateFlyweightPropertyJavadoc(sb, indent + INDENT, token, className);
        new Formatter(sb).format("\n" +
            indent + "    public %1$s %2$s()\n" +
            indent + "    {\n" +
            "%3$s" +
            indent + "        %2$s.wrap(buffer);\n" +
            "%4$s" +
            indent + "        return %2$s;\n" +
            indent + "    }\n",
            className,
            propertyName,
            actingVersionGuard,
            generateAccessOrderListenerCall(fieldPrecedenceModel, indent + "        ", token, propertyName + ".count"));
    }

    private void generateGroupEncoderProperty(
        final StringBuilder sb,
        final String groupName,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final Token token,
        final String indent,
        final boolean isSubGroup)
    {
        final String className = formatClassName(encoderName(groupName));
        final String propertyName = formatPropertyName(groupName);

        if (!isSubGroup)
        {
            new Formatter(sb).format("\n" +
                indent + "    private final %s %s = new %s(this);\n",
                className,
                propertyName,
                className);
        }

        new Formatter(sb).format("\n" +
            indent + "    public static long %sId()\n" +
            indent + "    {\n" +
            indent + "        return %d;\n" +
            indent + "    }\n",
            formatPropertyName(groupName),
            token.id());

        generateAccessOrderListenerMethodForGroupWrap(sb, "encode", fieldPrecedenceModel, indent + "    ", token);

        generateGroupEncodePropertyJavadoc(sb, indent + INDENT, token, className);
        new Formatter(sb).format("\n" +
            indent + "    public %1$s %2$sCount(final int count)\n" +
            indent + "    {\n" +
            "%3$s" +
            indent + "        %2$s.wrap(buffer, count);\n" +
            indent + "        return %2$s;\n" +
            indent + "    }\n",
            className,
            propertyName,
            generateAccessOrderListenerCall(fieldPrecedenceModel, indent + "        ", token, "count"));
    }

    private void generateDecoderVarData(
        final StringBuilder sb,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final List<Token> tokens,
        final String indent)
    {
        for (int i = 0, size = tokens.size(); i < size;)
        {
            final Token token = tokens.get(i);
            if (token.signal() != Signal.BEGIN_VAR_DATA)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_VAR_DATA: token=" + token);
            }

            generateFieldIdMethod(sb, token, indent);
            generateFieldSinceVersionMethod(sb, token, indent);

            final String characterEncoding = tokens.get(i + 3).encoding().characterEncoding();
            generateCharacterEncodingMethod(sb, token.name(), characterEncoding, indent);
            generateFieldMetaAttributeMethod(sb, token, indent);

            final String propertyName = Generators.toUpperFirstChar(token.name());
            final Token lengthToken = tokens.get(i + 2);
            final int sizeOfLengthField = lengthToken.encodedLength();
            final Encoding lengthEncoding = lengthToken.encoding();
            final PrimitiveType lengthType = lengthEncoding.primitiveType();
            final String byteOrderStr = byteOrderString(lengthEncoding);
            final String methodPropName = Generators.toLowerFirstChar(propertyName);

            sb.append("\n")
                .append(indent).append("    public static int ").append(methodPropName).append("HeaderLength()\n")
                .append(indent).append("    {\n")
                .append(indent).append("        return ").append(sizeOfLengthField).append(";\n")
                .append(indent).append("    }\n");

            generateAccessOrderListenerMethodForVarDataLength(sb, fieldPrecedenceModel, indent + "    ", token);

            final CharSequence lengthAccessOrderListenerCall = generateAccessOrderListenerCall(
                fieldPrecedenceModel, indent + "        ", accessOrderListenerMethodName(token, "Length"));

            generateAccessOrderListenerMethod(sb, fieldPrecedenceModel, indent + "    ", token);

            sb.append("\n")
                .append(indent).append("    public int ").append(methodPropName).append("Length()\n")
                .append(indent).append("    {\n")
                .append(generateArrayFieldNotPresentCondition(false, token.version(), indent))
                .append(lengthAccessOrderListenerCall)
                .append(indent).append("        final int limit = parentMessage.limit();\n")
                .append(indent).append("        return ").append(PrimitiveType.UINT32 == lengthType ? "(int)" : "")
                .append(generateGet(lengthType, "limit", byteOrderStr)).append(";\n")
                .append(indent).append("    }\n");

            final CharSequence accessOrderListenerCall =
                generateAccessOrderListenerCall(fieldPrecedenceModel, indent + "        ", token);

            generateDataDecodeMethods(sb, token, propertyName, sizeOfLengthField, lengthType,
                byteOrderStr, characterEncoding, accessOrderListenerCall, indent);

            i += token.componentTokenCount();
        }
    }

    private void generateEncoderVarData(
        final StringBuilder sb,
        final String className,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final List<Token> tokens,
        final String indent)
    {
        for (int i = 0, size = tokens.size(); i < size;)
        {
            final Token token = tokens.get(i);
            if (token.signal() != Signal.BEGIN_VAR_DATA)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_VAR_DATA: token=" + token);
            }

            generateFieldIdMethod(sb, token, indent);
            final Token varDataToken = Generators.findFirst("varData", tokens, i);
            final String characterEncoding = varDataToken.encoding().characterEncoding();
            generateCharacterEncodingMethod(sb, token.name(), characterEncoding, indent);
            generateFieldMetaAttributeMethod(sb, token, indent);

            final String propertyName = Generators.toUpperFirstChar(token.name());
            final Token lengthToken = Generators.findFirst("length", tokens, i);
            final int sizeOfLengthField = lengthToken.encodedLength();
            final Encoding lengthEncoding = lengthToken.encoding();
            final int maxLengthValue = (int)lengthEncoding.applicableMaxValue().longValue();
            final String byteOrderStr = byteOrderString(lengthEncoding);

            final String methodPropName = Generators.toLowerFirstChar(propertyName);
            sb.append("\n")
                .append(indent).append("    public static int ").append(methodPropName).append("HeaderLength()\n")
                .append(indent).append("    {\n")
                .append(indent).append("        return ")
                .append(sizeOfLengthField).append(";\n")
                .append(indent).append("    }\n");

            generateAccessOrderListenerMethod(sb, fieldPrecedenceModel, indent + "    ", token);
            final CharSequence accessOrderListenerCall =
                generateAccessOrderListenerCall(fieldPrecedenceModel, indent + "        ", token);

            generateDataEncodeMethods(
                sb,
                propertyName,
                accessOrderListenerCall,
                sizeOfLengthField,
                maxLengthValue,
                lengthEncoding.primitiveType(),
                byteOrderStr,
                characterEncoding,
                className,
                indent);

            i += token.componentTokenCount();
        }
    }

    private void generateDataDecodeMethods(
        final StringBuilder sb,
        final Token token,
        final String propertyName,
        final int sizeOfLengthField,
        final PrimitiveType lengthType,
        final String byteOrderStr,
        final String characterEncoding,
        final CharSequence accessOrderListenerCall,
        final String indent)
    {
        new Formatter(sb).format("\n" +
            indent + "    public int skip%1$s()\n" +
            indent + "    {\n" +
            "%2$s" +
            "%6$s" +
            indent + "        final int headerLength = %3$d;\n" +
            indent + "        final int limit = parentMessage.limit();\n" +
            indent + "        final int dataLength = %4$s%5$s;\n" +
            indent + "        final int dataOffset = limit + headerLength;\n" +
            indent + "        parentMessage.limit(dataOffset + dataLength);\n\n" +
            indent + "        return dataLength;\n" +
            indent + "    }\n",
            Generators.toUpperFirstChar(propertyName),
            generateStringNotPresentConditionForAppendable(false, token.version(), indent),
            sizeOfLengthField,
            PrimitiveType.UINT32 == lengthType ? "(int)" : "",
            generateGet(lengthType, "limit", byteOrderStr),
            accessOrderListenerCall);

        generateVarDataTypedDecoder(
            sb,
            token,
            propertyName,
            sizeOfLengthField,
            mutableBuffer,
            lengthType,
            byteOrderStr,
            accessOrderListenerCall,
            indent);

        generateVarDataTypedDecoder(
            sb,
            token,
            propertyName,
            sizeOfLengthField,
            "byte[]",
            lengthType,
            byteOrderStr,
            accessOrderListenerCall,
            indent);

        generateVarDataWrapDecoder(
            sb,
            token,
            propertyName,
            sizeOfLengthField,
            lengthType,
            byteOrderStr,
            accessOrderListenerCall,
            indent);

        if (null != characterEncoding)
        {
            new Formatter(sb).format("\n" +
                indent + "    public String %1$s()\n" +
                indent + "    {\n" +
                "%2$s" +
                "%7$s" +
                indent + "        final int headerLength = %3$d;\n" +
                indent + "        final int limit = parentMessage.limit();\n" +
                indent + "        final int dataLength = %4$s%5$s;\n" +
                indent + "        parentMessage.limit(limit + headerLength + dataLength);\n\n" +
                indent + "        if (0 == dataLength)\n" +
                indent + "        {\n" +
                indent + "            return \"\";\n" +
                indent + "        }\n\n" +
                indent + "        final byte[] tmp = new byte[dataLength];\n" +
                indent + "        buffer.getBytes(limit + headerLength, tmp, 0, dataLength);\n\n" +
                indent + "        return new String(tmp, %6$s);\n" +
                indent + "    }\n",
                formatPropertyName(propertyName),
                generateStringNotPresentCondition(false, token.version(), indent),
                sizeOfLengthField,
                PrimitiveType.UINT32 == lengthType ? "(int)" : "",
                generateGet(lengthType, "limit", byteOrderStr),
                charset(characterEncoding),
                accessOrderListenerCall);

            if (isAsciiEncoding(characterEncoding))
            {
                new Formatter(sb).format("\n" +
                    indent + "    public int get%1$s(final Appendable appendable)\n" +
                    indent + "    {\n" +
                    "%2$s" +
                    "%6$s" +
                    indent + "        final int headerLength = %3$d;\n" +
                    indent + "        final int limit = parentMessage.limit();\n" +
                    indent + "        final int dataLength = %4$s%5$s;\n" +
                    indent + "        final int dataOffset = limit + headerLength;\n\n" +
                    indent + "        parentMessage.limit(dataOffset + dataLength);\n" +
                    indent + "        buffer.getStringWithoutLengthAscii(dataOffset, dataLength, appendable);\n\n" +
                    indent + "        return dataLength;\n" +
                    indent + "    }\n",
                    Generators.toUpperFirstChar(propertyName),
                    generateStringNotPresentConditionForAppendable(false, token.version(), indent),
                    sizeOfLengthField,
                    PrimitiveType.UINT32 == lengthType ? "(int)" : "",
                    generateGet(lengthType, "limit", byteOrderStr),
                    accessOrderListenerCall);
            }
        }
    }

    private void generateVarDataWrapDecoder(
        final StringBuilder sb,
        final Token token,
        final String propertyName,
        final int sizeOfLengthField,
        final PrimitiveType lengthType,
        final String byteOrderStr,
        final CharSequence accessOrderListenerCall,
        final String indent)
    {
        new Formatter(sb).format("\n" +
            indent + "    public void wrap%s(final %s wrapBuffer)\n" +
            indent + "    {\n" +
            "%s" +
            "%s" +
            indent + "        final int headerLength = %d;\n" +
            indent + "        final int limit = parentMessage.limit();\n" +
            indent + "        final int dataLength = %s%s;\n" +
            indent + "        parentMessage.limit(limit + headerLength + dataLength);\n" +
            indent + "        wrapBuffer.wrap(buffer, limit + headerLength, dataLength);\n" +
            indent + "    }\n",
            propertyName,
            readOnlyBuffer,
            generateWrapFieldNotPresentCondition(token.version(), indent),
            accessOrderListenerCall,
            sizeOfLengthField,
            PrimitiveType.UINT32 == lengthType ? "(int)" : "",
            generateGet(lengthType, "limit", byteOrderStr));
    }

    private void generateDataEncodeMethods(
        final StringBuilder sb,
        final String propertyName,
        final CharSequence accessOrderListenerCall,
        final int sizeOfLengthField,
        final int maxLengthValue,
        final PrimitiveType lengthType,
        final String byteOrderStr,
        final String characterEncoding,
        final String className,
        final String indent)
    {
        generateDataTypedEncoder(
            sb,
            className,
            propertyName,
            accessOrderListenerCall,
            sizeOfLengthField,
            maxLengthValue,
            readOnlyBuffer,
            lengthType,
            byteOrderStr,
            indent);

        generateDataTypedEncoder(
            sb,
            className,
            propertyName,
            accessOrderListenerCall,
            sizeOfLengthField,
            maxLengthValue,
            "byte[]",
            lengthType,
            byteOrderStr,
            indent);

        if (null != characterEncoding)
        {
            generateCharArrayEncodeMethods(
                sb,
                propertyName,
                accessOrderListenerCall,
                sizeOfLengthField,
                maxLengthValue,
                lengthType,
                byteOrderStr,
                characterEncoding,
                className,
                indent);
        }
    }

    private void generateCharArrayEncodeMethods(
        final StringBuilder sb,
        final String propertyName,
        final CharSequence accessOrderListenerCall,
        final int sizeOfLengthField,
        final int maxLengthValue,
        final PrimitiveType lengthType,
        final String byteOrderStr,
        final String characterEncoding,
        final String className,
        final String indent)
    {
        final PrimitiveType lengthPutType = PrimitiveType.UINT32 == lengthType ? PrimitiveType.INT32 : lengthType;

        if (isAsciiEncoding(characterEncoding))
        {
            new Formatter(sb).format("\n" +
                indent + "    public %1$s %2$s(final String value)\n" +
                indent + "    {\n" +
                indent + "        final int length = null == value ? 0 : value.length();\n" +
                indent + "        if (length > %3$d)\n" +
                indent + "        {\n" +
                indent + "            throw new IllegalStateException(\"length > maxValue for type: \" + length);\n" +
                indent + "        }\n\n" +
                "%6$s" +
                indent + "        final int headerLength = %4$d;\n" +
                indent + "        final int limit = parentMessage.limit();\n" +
                indent + "        parentMessage.limit(limit + headerLength + length);\n" +
                indent + "        %5$s;\n" +
                indent + "        buffer.putStringWithoutLengthAscii(limit + headerLength, value);\n\n" +
                indent + "        return this;\n" +
                indent + "    }\n",
                className,
                formatPropertyName(propertyName),
                maxLengthValue,
                sizeOfLengthField,
                generatePut(lengthPutType, "limit", "length", byteOrderStr),
                accessOrderListenerCall);

            new Formatter(sb).format("\n" +
                indent + "    public %1$s %2$s(final CharSequence value)\n" +
                indent + "    {\n" +
                indent + "        final int length = null == value ? 0 : value.length();\n" +
                indent + "        if (length > %3$d)\n" +
                indent + "        {\n" +
                indent + "            throw new IllegalStateException(\"length > maxValue for type: \" + length);\n" +
                indent + "        }\n\n" +
                "%6$s" +
                indent + "        final int headerLength = %4$d;\n" +
                indent + "        final int limit = parentMessage.limit();\n" +
                indent + "        parentMessage.limit(limit + headerLength + length);\n" +
                indent + "        %5$s;\n" +
                indent + "        buffer.putStringWithoutLengthAscii(limit + headerLength, value);\n\n" +
                indent + "        return this;\n" +
                indent + "    }\n",
                className,
                formatPropertyName(propertyName),
                maxLengthValue,
                sizeOfLengthField,
                generatePut(lengthPutType, "limit", "length", byteOrderStr),
                accessOrderListenerCall);
        }
        else
        {
            new Formatter(sb).format("\n" +
                indent + "    public %1$s %2$s(final String value)\n" +
                indent + "    {\n" +
                indent + "        final byte[] bytes = (null == value || value.isEmpty()) ?" +
                " org.agrona.collections.ArrayUtil.EMPTY_BYTE_ARRAY : value.getBytes(%3$s);\n\n" +
                indent + "        final int length = bytes.length;\n" +
                indent + "        if (length > %4$d)\n" +
                indent + "        {\n" +
                indent + "            throw new IllegalStateException(\"length > maxValue for type: \" + length);\n" +
                indent + "        }\n\n" +
                "%7$s" +
                indent + "        final int headerLength = %5$d;\n" +
                indent + "        final int limit = parentMessage.limit();\n" +
                indent + "        parentMessage.limit(limit + headerLength + length);\n" +
                indent + "        %6$s;\n" +
                indent + "        buffer.putBytes(limit + headerLength, bytes, 0, length);\n\n" +
                indent + "        return this;\n" +
                indent + "    }\n",
                className,
                formatPropertyName(propertyName),
                charset(characterEncoding),
                maxLengthValue,
                sizeOfLengthField,
                generatePut(lengthPutType, "limit", "length", byteOrderStr),
                accessOrderListenerCall);
        }
    }

    private void generateVarDataTypedDecoder(
        final StringBuilder sb,
        final Token token,
        final String propertyName,
        final int sizeOfLengthField,
        final String exchangeType,
        final PrimitiveType lengthType,
        final String byteOrderStr,
        final CharSequence accessOrderListenerCall,
        final String indent)
    {
        new Formatter(sb).format("\n" +
            indent + "    public int get%s(final %s dst, final int dstOffset, final int length)\n" +
            indent + "    {\n" +
            "%s" +
            "%s" +
            indent + "        final int headerLength = %d;\n" +
            indent + "        final int limit = parentMessage.limit();\n" +
            indent + "        final int dataLength = %s%s;\n" +
            indent + "        final int bytesCopied = Math.min(length, dataLength);\n" +
            indent + "        parentMessage.limit(limit + headerLength + dataLength);\n" +
            indent + "        buffer.getBytes(limit + headerLength, dst, dstOffset, bytesCopied);\n\n" +
            indent + "        return bytesCopied;\n" +
            indent + "    }\n",
            propertyName,
            exchangeType,
            generateArrayFieldNotPresentCondition(false, token.version(), indent),
            accessOrderListenerCall,
            sizeOfLengthField,
            PrimitiveType.UINT32 == lengthType ? "(int)" : "",
            generateGet(lengthType, "limit", byteOrderStr));
    }

    private void generateDataTypedEncoder(
        final StringBuilder sb,
        final String className,
        final String propertyName,
        final CharSequence accessOrderListenerCall,
        final int sizeOfLengthField,
        final int maxLengthValue,
        final String exchangeType,
        final PrimitiveType lengthType,
        final String byteOrderStr,
        final String indent)
    {
        final PrimitiveType lengthPutType = PrimitiveType.UINT32 == lengthType ? PrimitiveType.INT32 : lengthType;

        new Formatter(sb).format("\n" +
            indent + "    public %1$s put%2$s(final %3$s src, final int srcOffset, final int length)\n" +
            indent + "    {\n" +
            indent + "        if (length > %4$d)\n" +
            indent + "        {\n" +
            indent + "            throw new IllegalStateException(\"length > maxValue for type: \" + length);\n" +
            indent + "        }\n\n" +
            "%7$s" +
            indent + "        final int headerLength = %5$d;\n" +
            indent + "        final int limit = parentMessage.limit();\n" +
            indent + "        parentMessage.limit(limit + headerLength + length);\n" +
            indent + "        %6$s;\n" +
            indent + "        buffer.putBytes(limit + headerLength, src, srcOffset, length);\n\n" +
            indent + "        return this;\n" +
            indent + "    }\n",
            className,
            propertyName,
            exchangeType,
            maxLengthValue,
            sizeOfLengthField,
            generatePut(lengthPutType, "limit", "length", byteOrderStr),
            accessOrderListenerCall);
    }

    private void generateBitSet(final List<Token> tokens) throws IOException
    {
        final Token token = tokens.get(0);
        final String bitSetName = token.applicableTypeName();
        final String decoderName = decoderName(bitSetName);
        final String encoderName = encoderName(bitSetName);
        final List<Token> choiceList = tokens.subList(1, tokens.size() - 1);
        final String implementsString = implementsInterface(Flyweight.class.getSimpleName());

        registerTypesPackageName(token, ir);
        try (Writer out = outputManager.createOutput(decoderName))
        {
            final Encoding encoding = token.encoding();
            generateFixedFlyweightHeader(
                out, token, decoderName, implementsString, readOnlyBuffer, fqReadOnlyBuffer, PACKAGES_EMPTY_SET);
            out.append(generateChoiceIsEmpty(encoding.primitiveType()));

            new Formatter(out).format(
                "\n" +
                "    public %s getRaw()\n" +
                "    {\n" +
                "        return %s;\n" +
                "    }\n",
                primitiveTypeName(token),
                generateGet(encoding.primitiveType(), "offset", byteOrderString(encoding)));

            generateChoiceDecoders(out, choiceList);
            out.append(generateChoiceDisplay(choiceList));
            out.append("}\n");
        }

        registerTypesPackageName(token, ir);
        try (Writer out = outputManager.createOutput(encoderName))
        {
            generateFixedFlyweightHeader(
                out, token, encoderName, implementsString, mutableBuffer, fqMutableBuffer, PACKAGES_EMPTY_SET);
            generateChoiceClear(out, encoderName, token);
            generateChoiceEncoders(out, encoderName, choiceList);
            out.append("}\n");
        }
    }

    private void generateFixedFlyweightHeader(
        final Writer out,
        final Token token,
        final String typeName,
        final String implementsString,
        final String buffer,
        final String fqBuffer,
        final Set<String> importedTypesPackages) throws IOException
    {
        out.append(generateFileHeader(registerTypesPackageName(token, ir), importedTypesPackages, fqBuffer));
        out.append(generateDeclaration(typeName, implementsString, token));
        out.append(generateFixedFlyweightCode(typeName, token.encodedLength(), buffer));
    }

    private void generateCompositeFlyweightHeader(
        final Token token,
        final String typeName,
        final Writer out,
        final String buffer,
        final String fqBuffer,
        final String implementsString,
        final Set<String> importedTypesPackages) throws IOException
    {
        out.append(generateFileHeader(registerTypesPackageName(token, ir), importedTypesPackages, fqBuffer));
        out.append(generateDeclaration(typeName, implementsString, token));
        out.append(generateFixedFlyweightCode(typeName, token.encodedLength(), buffer));
    }

    private void generateEnum(final List<Token> tokens) throws IOException
    {
        final Token enumToken = tokens.get(0);
        final String enumName = formatClassName(enumToken.applicableTypeName());
        final Encoding encoding = enumToken.encoding();
        final String nullVal = encoding.applicableNullValue().toString();
        final String packageName = registerTypesPackageName(enumToken, ir);

        try (Writer out = outputManager.createOutput(enumName))
        {
            out.append(generateEnumFileHeader(packageName));
            out.append(generateEnumDeclaration(enumName, enumToken));

            final List<Token> valuesList = tokens.subList(1, tokens.size() - 1);
            out.append(generateEnumValues(valuesList, generateLiteral(encoding.primitiveType(), nullVal)));
            out.append(generateEnumBody(enumToken, enumName));

            out.append(generateEnumLookupMethod(valuesList, enumName, nullVal));

            out.append("}\n");
        }
    }

    private void generateComposite(final List<Token> tokens) throws IOException
    {
        final Token token = tokens.get(0);
        final String compositeName = token.applicableTypeName();
        final String decoderName = decoderName(compositeName);
        final String encoderName = encoderName(compositeName);

        registerTypesPackageName(token, ir);
        final Set<String> importedTypesPackages = scanPackagesToImport(tokens);

        try (Writer out = outputManager.createOutput(decoderName))
        {
            final String implementsString = implementsInterface(CompositeDecoderFlyweight.class.getSimpleName());
            generateCompositeFlyweightHeader(
                token, decoderName, out, readOnlyBuffer, fqReadOnlyBuffer, implementsString, importedTypesPackages);

            for (int i = 1, end = tokens.size() - 1; i < end;)
            {
                final Token encodingToken = tokens.get(i);
                final String propertyName = formatPropertyName(encodingToken.name());
                final String typeName = decoderName(encodingToken.applicableTypeName());

                final StringBuilder sb = new StringBuilder();
                generateEncodingOffsetMethod(sb, propertyName, encodingToken.offset(), BASE_INDENT);
                generateEncodingLengthMethod(sb, propertyName, encodingToken.encodedLength(), BASE_INDENT);
                generateFieldSinceVersionMethod(sb, encodingToken, BASE_INDENT);
                final String accessOrderListenerCall = "";

                switch (encodingToken.signal())
                {
                    case ENCODING:
                        generatePrimitiveDecoder(
                            sb, true, encodingToken.name(), "", encodingToken, encodingToken, BASE_INDENT);
                        break;

                    case BEGIN_ENUM:
                        generateEnumDecoder(sb, true, "", encodingToken, propertyName, encodingToken, BASE_INDENT);
                        break;

                    case BEGIN_SET:
                        generateBitSetProperty(
                            sb, true, DECODER, propertyName, accessOrderListenerCall,
                            encodingToken, encodingToken, BASE_INDENT, typeName);
                        break;

                    case BEGIN_COMPOSITE:
                        generateCompositeProperty(
                            sb, true, DECODER, propertyName, accessOrderListenerCall,
                            encodingToken, encodingToken, BASE_INDENT, typeName);
                        break;

                    default:
                        break;
                }

                out.append(sb);
                i += encodingToken.componentTokenCount();
            }

            out.append(generateCompositeDecoderDisplay(tokens));

            out.append("}\n");
        }

        registerTypesPackageName(token, ir);
        try (Writer out = outputManager.createOutput(encoderName))
        {
            final String implementsString = implementsInterface(CompositeEncoderFlyweight.class.getSimpleName());
            generateCompositeFlyweightHeader(
                token, encoderName, out, mutableBuffer, fqMutableBuffer, implementsString, importedTypesPackages);

            for (int i = 1, end = tokens.size() - 1; i < end;)
            {
                final Token encodingToken = tokens.get(i);
                final String propertyName = formatPropertyName(encodingToken.name());
                final String typeName = encoderName(encodingToken.applicableTypeName());

                final StringBuilder sb = new StringBuilder();
                generateEncodingOffsetMethod(sb, propertyName, encodingToken.offset(), BASE_INDENT);
                generateEncodingLengthMethod(sb, propertyName, encodingToken.encodedLength(), BASE_INDENT);
                final String accessOrderListenerCall = "";

                switch (encodingToken.signal())
                {
                    case ENCODING:
                        generatePrimitiveEncoder(sb, encoderName, encodingToken.name(),
                            accessOrderListenerCall, encodingToken, BASE_INDENT);
                        break;

                    case BEGIN_ENUM:
                        generateEnumEncoder(sb, encoderName, accessOrderListenerCall,
                            encodingToken, propertyName, encodingToken, BASE_INDENT);
                        break;

                    case BEGIN_SET:
                        generateBitSetProperty(
                            sb, true, ENCODER, propertyName, accessOrderListenerCall,
                            encodingToken, encodingToken, BASE_INDENT, typeName);
                        break;

                    case BEGIN_COMPOSITE:
                        generateCompositeProperty(
                            sb, true, ENCODER, propertyName, accessOrderListenerCall,
                            encodingToken, encodingToken, BASE_INDENT, typeName);
                        break;

                    default:
                        break;
                }

                out.append(sb);
                i += encodingToken.componentTokenCount();
            }

            out.append(generateCompositeEncoderDisplay(decoderName));
            out.append("}\n");
        }
    }

    private Set<String> scanPackagesToImport(final List<Token> tokens)
    {
        if (!shouldSupportTypesPackageNames)
        {
            return PACKAGES_EMPTY_SET;
        }

        final Set<String> packagesToImport = new HashSet<>();

        for (int i = 1, limit = tokens.size() - 1; i < limit; i++)
        {
            final Token typeToken = tokens.get(i);
            if (typeToken.signal() == Signal.BEGIN_ENUM ||
                typeToken.signal() == Signal.BEGIN_SET ||
                typeToken.signal() == Signal.BEGIN_COMPOSITE)
            {
                if (typeToken.packageName() != null)
                {
                    packagesToImport.add(typeToken.packageName());
                }
            }
        }

        return packagesToImport;
    }

    private void generateChoiceClear(final Appendable out, final String bitSetClassName, final Token token)
        throws IOException
    {
        final Encoding encoding = token.encoding();
        final String literalValue = generateLiteral(encoding.primitiveType(), "0");
        final String byteOrderStr = byteOrderString(encoding);

        final String clearStr = generatePut(encoding.primitiveType(), "offset", literalValue, byteOrderStr);
        out.append("\n")
            .append("    public ").append(bitSetClassName).append(" clear()\n")
            .append("    {\n")
            .append("        ").append(clearStr).append(";\n")
            .append("        return this;\n")
            .append("    }\n");
    }

    private void generateChoiceDecoders(final Appendable out, final List<Token> tokens)
        throws IOException
    {
        for (final Token token : tokens)
        {
            if (token.signal() == Signal.CHOICE)
            {
                final String choiceName = formatPropertyName(token.name());
                final Encoding encoding = token.encoding();
                final String choiceBitIndex = encoding.constValue().toString();
                final String byteOrderStr = byteOrderString(encoding);
                final PrimitiveType primitiveType = encoding.primitiveType();
                final String argType = bitsetArgType(primitiveType);

                generateOptionDecodeJavadoc(out, INDENT, token);
                final String choiceGet = generateChoiceGet(primitiveType, choiceBitIndex, byteOrderStr);
                final String staticChoiceGet = generateStaticChoiceGet(primitiveType, choiceBitIndex);
                out.append("\n")
                    .append("    public boolean ").append(choiceName).append("()\n")
                    .append("    {\n")
                    .append("        return ").append(choiceGet).append(";\n")
                    .append("    }\n\n")
                    .append("    public static boolean ").append(choiceName)
                    .append("(final ").append(argType).append(" value)\n")
                    .append("    {\n").append("        return ").append(staticChoiceGet).append(";\n")
                    .append("    }\n");
            }
        }
    }

    private void generateChoiceEncoders(final Appendable out, final String bitSetClassName, final List<Token> tokens)
        throws IOException
    {
        for (final Token token : tokens)
        {
            if (token.signal() == Signal.CHOICE)
            {
                final String choiceName = formatPropertyName(token.name());
                final Encoding encoding = token.encoding();
                final String choiceBitIndex = encoding.constValue().toString();
                final String byteOrderStr = byteOrderString(encoding);
                final PrimitiveType primitiveType = encoding.primitiveType();
                final String argType = bitsetArgType(primitiveType);

                generateOptionEncodeJavadoc(out, INDENT, token);
                final String choicePut = generateChoicePut(encoding.primitiveType(), choiceBitIndex, byteOrderStr);
                final String staticChoicePut = generateStaticChoicePut(encoding.primitiveType(), choiceBitIndex);
                out.append("\n")
                    .append("    public ").append(bitSetClassName).append(" ").append(choiceName)
                    .append("(final boolean value)\n")
                    .append("    {\n")
                    .append(choicePut).append("\n")
                    .append("        return this;\n")
                    .append("    }\n\n")
                    .append("    public static ").append(argType).append(" ").append(choiceName)
                    .append("(final ").append(argType).append(" bits, final boolean value)\n")
                    .append("    {\n")
                    .append(staticChoicePut)
                    .append("    }\n");
            }
        }
    }

    private String bitsetArgType(final PrimitiveType primitiveType)
    {
        switch (primitiveType)
        {
            case UINT8:
                return "byte";

            case UINT16:
                return "short";

            case UINT32:
                return "int";

            case UINT64:
                return "long";

            default:
                throw new IllegalStateException("Invalid type: " + primitiveType);
        }
    }

    private CharSequence generateEnumValues(final List<Token> tokens, final String nullVal)
    {
        final StringBuilder sb = new StringBuilder();

        for (final Token token : tokens)
        {
            final Encoding encoding = token.encoding();
            final CharSequence constVal = generateLiteral(encoding.primitiveType(), encoding.constValue().toString());
            generateTypeJavadoc(sb, INDENT, token);
            sb.append(INDENT).append(token.name()).append('(').append(constVal).append("),\n\n");
        }

        if (shouldDecodeUnknownEnumValues)
        {
            sb.append(INDENT).append("/**\n");
            sb.append(INDENT).append(" * To be used to represent an unknown value from a later version.\n");
            sb.append(INDENT).append(" */\n");
            sb.append(INDENT).append("SBE_UNKNOWN").append('(').append(nullVal).append("),\n\n");
        }

        sb.append(INDENT).append("/**\n");
        sb.append(INDENT).append(" * To be used to represent not present or null.\n");
        sb.append(INDENT).append(" */\n");
        sb.append(INDENT).append("NULL_VAL").append('(').append(nullVal).append(");\n\n");

        return sb;
    }

    private CharSequence generateEnumBody(final Token token, final String enumName)
    {
        final String javaEncodingType = primitiveTypeName(token);

        return
            "    private final " + javaEncodingType + " value;\n\n" +
            "    " + enumName + "(final " + javaEncodingType + " value)\n" +
            "    {\n" +
            "        this.value = value;\n" +
            "    }\n\n" +
            "    /**\n" +
            "     * The raw encoded value in the Java type representation.\n" +
            "     *\n" +
            "     * @return the raw value encoded.\n" +
            "     */\n" +
            "    public " + javaEncodingType + " value()\n" +
            "    {\n" +
            "        return value;\n" +
            "    }\n";
    }

    private CharSequence generateEnumLookupMethod(final List<Token> tokens, final String enumName, final String nullVal)
    {
        final StringBuilder sb = new StringBuilder();
        final PrimitiveType primitiveType = tokens.get(0).encoding().primitiveType();

        sb.append("\n")
            .append("    /**\n")
            .append("     * Lookup the enum value representing the value.\n")
            .append("     *\n")
            .append("     * @param value encoded to be looked up.\n")
            .append("     * @return the enum value representing the value.\n")
            .append("     */\n")
            .append("    public static ").append(enumName)
            .append(" get(final ").append(javaTypeName(primitiveType)).append(" value)\n").append("    {\n")
            .append("        switch (value)\n").append("        {\n");

        for (final Token token : tokens)
        {
            final String constStr = token.encoding().constValue().toString();
            final String name = token.name();
            sb.append("            case ").append(constStr).append(": return ").append(name).append(";\n");
        }

        sb.append("            case ").append(nullVal).append(": return NULL_VAL").append(";\n");

        final String handleUnknownLogic = shouldDecodeUnknownEnumValues ?
            INDENT + INDENT + "return SBE_UNKNOWN;\n" :
            INDENT + INDENT + "throw new IllegalArgumentException(\"Unknown value: \" + value);\n";

        sb.append("        }\n\n")
            .append(handleUnknownLogic)
            .append("    }\n");

        return sb;
    }

    private StringBuilder generateImportStatements(final Set<String> packages, final String currentPackage)
    {
        final StringBuilder importStatements = new StringBuilder();

        for (final String candidatePackage : packages)
        {
            if (!candidatePackage.equals(currentPackage))
            {
                importStatements.append("import ").append(candidatePackage).append(".*;\n");
            }
        }

        if (importStatements.length() > 0)
        {
            importStatements.append("\n\n");
        }

        return importStatements;
    }

    private String interfaceImportLine()
    {
        if (!shouldGenerateInterfaces)
        {
            return "\n";
        }

        return "import " + JAVA_INTERFACE_PACKAGE + ".*;\n\n";
    }


    private CharSequence generateFileHeader(final String packageName, final Set<String> importedTypesPackages,
        final String fqBuffer)
    {
        final StringBuilder importStatements = generateImportStatements(importedTypesPackages, packageName);

        return "/* Generated SBE (Simple Binary Encoding) message codec. */\n" +
            "package " + packageName + ";\n\n" +
            "import " + fqBuffer + ";\n" +
            interfaceImportLine() +
            importStatements;
    }

    private CharSequence generateMainHeader(
        final String packageName, final CodecType codecType, final boolean hasVarData)
    {
        final StringBuilder importStatements = generateImportStatements(packageNameByTypes, packageName);

        if (fqMutableBuffer.equals(fqReadOnlyBuffer))
        {
            return
                "/* Generated SBE (Simple Binary Encoding) message codec. */\n" +
                "package " + packageName + ";\n\n" +
                "import " + fqMutableBuffer + ";\n" +
                interfaceImportLine() +
                importStatements;
        }
        else
        {
            final boolean hasMutableBuffer = ENCODER == codecType || hasVarData;
            final boolean hasReadOnlyBuffer = DECODER == codecType || hasVarData;

            return
                "/* Generated SBE (Simple Binary Encoding) message codec. */\n" +
                "package " + packageName + ";\n\n" +
                (hasMutableBuffer ? "import " + fqMutableBuffer + ";\n" : "") +
                (hasReadOnlyBuffer ? "import " + fqReadOnlyBuffer + ";\n" : "") +
                interfaceImportLine() +
                importStatements;
        }
    }

    private static CharSequence generateEnumFileHeader(final String packageName)
    {
        return
            "/* Generated SBE (Simple Binary Encoding) message codec. */\n" +
            "package " + packageName + ";\n\n";
    }

    private void generateAnnotations(
        final String indent,
        final String className,
        final List<Token> tokens,
        final Appendable out,
        final Function<String, String> nameMapping) throws IOException
    {
        final List<String> groupClassNames = new ArrayList<>();
        int level = 0;

        for (final Token token : tokens)
        {
            if (token.signal() == Signal.BEGIN_GROUP)
            {
                if (1 == ++level)
                {
                    groupClassNames.add(formatClassName(nameMapping.apply(token.name())));
                }
            }
            else if (token.signal() == Signal.END_GROUP)
            {
                --level;
            }
        }

        if (!groupClassNames.isEmpty())
        {
            out.append(indent).append("@uk.co.real_logic.sbe.codec.java.GroupOrder({\n");
            int i = 0;
            for (final String name : groupClassNames)
            {
                out.append(indent).append(INDENT).append(className).append('.').append(name).append(".class");
                if (++i < groupClassNames.size())
                {
                    out.append(",\n");
                }
            }

            out.append("})");
        }
    }

    private static CharSequence generateDeclaration(
        final String className, final String implementsString, final Token typeToken)
    {
        final StringBuilder sb = new StringBuilder();

        generateTypeJavadoc(sb, BASE_INDENT, typeToken);
        if (typeToken.deprecated() > 0)
        {
            sb.append("@Deprecated\n");
        }
        sb.append("@SuppressWarnings(\"all\")\n")
            .append("public final class ").append(className).append(implementsString).append('\n')
            .append("{\n");

        return sb;
    }

    private void generatePackageInfo() throws IOException
    {
        try (Writer out = outputManager.createOutput(PACKAGE_INFO))
        {
            out.append(
                "/* Generated SBE (Simple Binary Encoding) message codecs.*/\n" +
                "/**\n" +
                " * ").append(ir.description()).append("\n")
                .append(
                " */\n" +
                "package ").append(ir.applicableNamespace()).append(";\n");
        }
    }

    private void generateMetaAttributeEnum() throws IOException
    {
        try (Writer out = outputManager.createOutput(META_ATTRIBUTE_ENUM))
        {
            out.append(
                "/* Generated SBE (Simple Binary Encoding) message codec. */\n" +
                "package ").append(ir.applicableNamespace()).append(";\n\n")
                .append(
                "/**\n" +
                " * Meta attribute enum for selecting a particular meta attribute value.\n" +
                " */\n" +
                " @SuppressWarnings(\"all\")\n" +
                "public enum MetaAttribute\n" +
                "{\n" +
                "    /**\n" +
                "     * The epoch or start of time. Default is 'UNIX' which is midnight 1st January 1970 UTC.\n" +
                "     */\n" +
                "    EPOCH,\n\n" +
                "    /**\n" +
                "     * Time unit applied to the epoch. Can be second, millisecond, microsecond, or nanosecond.\n" +
                "     */\n" +
                "    TIME_UNIT,\n\n" +
                "    /**\n" +
                "     * The type relationship to a FIX tag value encoded type. For reference only.\n" +
                "     */\n" +
                "    SEMANTIC_TYPE,\n\n" +
                "    /**\n" +
                "     * Field presence indication. Can be optional, required, or constant.\n" +
                "     */\n" +
                "    PRESENCE\n" +
                "}\n");
        }
    }

    private static CharSequence generateEnumDeclaration(final String name, final Token typeToken)
    {
        final StringBuilder sb = new StringBuilder();

        generateTypeJavadoc(sb, BASE_INDENT, typeToken);
        sb.append("@SuppressWarnings(\"all\")\n").append("public enum ").append(name).append("\n{\n");

        return sb;
    }

    private void generatePrimitiveDecoder(
        final StringBuilder sb,
        final boolean inComposite,
        final String propertyName,
        final CharSequence accessOrderListenerCall,
        final Token propertyToken,
        final Token encodingToken,
        final String indent)
    {
        final String formattedPropertyName = formatPropertyName(propertyName);

        generatePrimitiveFieldMetaMethod(sb, formattedPropertyName, encodingToken, indent);

        if (encodingToken.isConstantEncoding())
        {
            generateConstPropertyMethods(sb, formattedPropertyName, encodingToken, indent);
        }
        else
        {
            sb.append(generatePrimitivePropertyDecodeMethods(
                inComposite, formattedPropertyName, accessOrderListenerCall, propertyToken, encodingToken, indent));
        }
    }

    private void generatePrimitiveEncoder(
        final StringBuilder sb,
        final String containingClassName,
        final String propertyName,
        final CharSequence accessOrderListenerCall,
        final Token typeToken,
        final String indent)
    {
        final String formattedPropertyName = formatPropertyName(propertyName);

        generatePrimitiveFieldMetaMethod(sb, formattedPropertyName, typeToken, indent);

        if (!typeToken.isConstantEncoding())
        {
            sb.append(generatePrimitivePropertyEncodeMethods(
                containingClassName, formattedPropertyName, accessOrderListenerCall, typeToken, indent));
        }
        else
        {
            generateConstPropertyMethods(sb, formattedPropertyName, typeToken, indent);
        }
    }

    private CharSequence generatePrimitivePropertyDecodeMethods(
        final boolean inComposite,
        final String propertyName,
        final CharSequence accessOrderListenerCall,
        final Token propertyToken,
        final Token encodingToken,
        final String indent)
    {
        return encodingToken.matchOnLength(
            () -> generatePrimitivePropertyDecode(
                inComposite, propertyName, accessOrderListenerCall, propertyToken, encodingToken, indent),
            () -> generatePrimitiveArrayPropertyDecode(
                inComposite, propertyName, accessOrderListenerCall, propertyToken, encodingToken, indent));
    }

    private CharSequence generatePrimitivePropertyEncodeMethods(
        final String containingClassName,
        final String propertyName,
        final CharSequence accessOrderListenerCall,
        final Token typeToken,
        final String indent)
    {
        return typeToken.matchOnLength(
            () -> generatePrimitivePropertyEncode(
                containingClassName, propertyName, accessOrderListenerCall, typeToken, indent),
            () -> generatePrimitiveArrayPropertyEncode(
                containingClassName, propertyName, accessOrderListenerCall, typeToken, indent));
    }

    private void generatePrimitiveFieldMetaMethod(
        final StringBuilder sb, final String propertyName, final Token token, final String indent)
    {
        final PrimitiveType primitiveType = token.encoding().primitiveType();
        final String javaTypeName = javaTypeName(primitiveType);
        final String formattedPropertyName = formatPropertyName(propertyName);

        final String nullValue = generateLiteral(primitiveType, token.encoding().applicableNullValue().toString());
        generatePrimitiveFieldMetaMethod(sb, indent, javaTypeName, formattedPropertyName, "Null", nullValue);

        final String minValue = generateLiteral(primitiveType, token.encoding().applicableMinValue().toString());
        generatePrimitiveFieldMetaMethod(sb, indent, javaTypeName, formattedPropertyName, "Min", minValue);

        final String maxValue = generateLiteral(primitiveType, token.encoding().applicableMaxValue().toString());
        generatePrimitiveFieldMetaMethod(sb, indent, javaTypeName, formattedPropertyName, "Max", maxValue);
    }

    private void generatePrimitiveFieldMetaMethod(
        final StringBuilder sb,
        final String indent,
        final String javaTypeName,
        final String formattedPropertyName,
        final String metaType,
        final String retValue)
    {
        sb.append("\n")
            .append(indent).append("    public static ")
            .append(javaTypeName).append(" ").append(formattedPropertyName).append(metaType).append("Value()\n")
            .append(indent).append("    {\n")
            .append(indent).append("        return ").append(retValue).append(";\n")
            .append(indent).append("    }\n");
    }

    private CharSequence generatePrimitivePropertyDecode(
        final boolean inComposite,
        final String propertyName,
        final CharSequence accessOrderListenerCall,
        final Token propertyToken,
        final Token encodingToken,
        final String indent)
    {
        final Encoding encoding = encodingToken.encoding();
        final String javaTypeName = javaTypeName(encoding.primitiveType());

        final int offset = encodingToken.offset();
        final String byteOrderStr = byteOrderString(encoding);

        return String.format(
            "\n" +
            indent + "    public %s %s()\n" +
            indent + "    {\n" +
            "%s" +
            "%s" +
            indent + "        return %s;\n" +
            indent + "    }\n\n",
            javaTypeName,
            formatPropertyName(propertyName),
            generateFieldNotPresentCondition(inComposite, propertyToken.version(), encoding, indent),
            accessOrderListenerCall,
            generateGet(encoding.primitiveType(), "offset + " + offset, byteOrderStr));
    }

    private CharSequence generatePrimitivePropertyEncode(
        final String containingClassName,
        final String propertyName,
        final CharSequence accessOrderListenerCall,
        final Token typeToken,
        final String indent)
    {
        final Encoding encoding = typeToken.encoding();
        final String javaTypeName = javaTypeName(encoding.primitiveType());
        final int offset = typeToken.offset();
        final String byteOrderStr = byteOrderString(encoding);

        return String.format(
            "\n" +
            indent + "    public %s %s(final %s value)\n" +
            indent + "    {\n" +
            "%s" +
            indent + "        %s;\n" +
            indent + "        return this;\n" +
            indent + "    }\n\n",
            formatClassName(containingClassName),
            formatPropertyName(propertyName),
            javaTypeName,
            accessOrderListenerCall,
            generatePut(encoding.primitiveType(), "offset + " + offset, "value", byteOrderStr));
    }

    private CharSequence generateWrapFieldNotPresentCondition(final int sinceVersion, final String indent)
    {
        if (0 == sinceVersion)
        {
            return "";
        }

        return
            indent + "        if (parentMessage.actingVersion < " + sinceVersion + ")\n" +
            indent + "        {\n" +
            indent + "            wrapBuffer.wrap(buffer, offset, 0);\n" +
            indent + "            return;\n" +
            indent + "        }\n\n";
    }

    private CharSequence generateFieldNotPresentCondition(
        final boolean inComposite, final int sinceVersion, final Encoding encoding, final String indent)
    {
        if (inComposite || 0 == sinceVersion)
        {
            return "";
        }

        final String nullValue = generateLiteral(encoding.primitiveType(), encoding.applicableNullValue().toString());
        return
            indent + "        if (parentMessage.actingVersion < " + sinceVersion + ")\n" +
            indent + "        {\n" +
            indent + "            return " + nullValue + ";\n" +
            indent + "        }\n\n";
    }

    private static CharSequence generateArrayFieldNotPresentCondition(
        final boolean inComposite, final int sinceVersion, final String indent)
    {
        if (inComposite || 0 == sinceVersion)
        {
            return "";
        }

        return
            indent + "        if (parentMessage.actingVersion < " + sinceVersion + ")\n" +
            indent + "        {\n" +
            indent + "            return 0;\n" +
            indent + "        }\n\n";
    }

    private static CharSequence generateStringNotPresentConditionForAppendable(
        final boolean inComposite, final int sinceVersion, final String indent)
    {
        if (inComposite || 0 == sinceVersion)
        {
            return "";
        }

        return
            indent + "        if (parentMessage.actingVersion < " + sinceVersion + ")\n" +
            indent + "        {\n" +
            indent + "            return 0;\n" +
            indent + "        }\n\n";
    }

    private static CharSequence generateStringNotPresentCondition(
        final boolean inComposite, final int sinceVersion, final String indent)
    {
        if (inComposite || 0 == sinceVersion)
        {
            return "";
        }

        return
            indent + "        if (parentMessage.actingVersion < " + sinceVersion + ")\n" +
            indent + "        {\n" +
            indent + "            return \"\";\n" +
            indent + "        }\n\n";
    }

    private static CharSequence generatePropertyNotPresentCondition(
        final boolean inComposite,
        final CodecType codecType,
        final Token propertyToken,
        final String enumName,
        final String indent)
    {
        if (inComposite || codecType == ENCODER || 0 == propertyToken.version())
        {
            return "";
        }

        final String nullValue = enumName == null ? "null" : (enumName + ".NULL_VAL");
        return
            indent + "        if (parentMessage.actingVersion < " + propertyToken.version() + ")\n" +
            indent + "        {\n" +
            indent + "            return " + nullValue + ";\n" +
            indent + "        }\n\n";
    }

    private CharSequence generatePrimitiveArrayPropertyDecode(
        final boolean inComposite,
        final String propertyName,
        final CharSequence accessOrderListenerCall,
        final Token propertyToken,
        final Token encodingToken,
        final String indent)
    {
        final Encoding encoding = encodingToken.encoding();
        final String javaTypeName = javaTypeName(encoding.primitiveType());
        final int offset = encodingToken.offset();
        final String byteOrderStr = byteOrderString(encoding);
        final int fieldLength = encodingToken.arrayLength();
        final int typeSize = sizeOfPrimitive(encoding);

        final StringBuilder sb = new StringBuilder();

        generateArrayLengthMethod(propertyName, indent, fieldLength, sb);

        new Formatter(sb).format("\n" +
            indent + "    public %s %s(final int index)\n" +
            indent + "    {\n" +
            indent + "        if (index < 0 || index >= %d)\n" +
            indent + "        {\n" +
            indent + "            throw new IndexOutOfBoundsException(\"index out of range: index=\" + index);\n" +
            indent + "        }\n\n" +
            "%s" +
            "%s" +
            indent + "        final int pos = offset + %d + (index * %d);\n\n" +
            indent + "        return %s;\n" +
            indent + "    }\n\n",
            javaTypeName,
            propertyName,
            fieldLength,
            generateFieldNotPresentCondition(inComposite, propertyToken.version(), encoding, indent),
            accessOrderListenerCall,
            offset,
            typeSize,
            generateGet(encoding.primitiveType(), "pos", byteOrderStr));

        if (encoding.primitiveType() == PrimitiveType.CHAR)
        {
            generateCharacterEncodingMethod(sb, propertyName, encoding.characterEncoding(), indent);

            new Formatter(sb).format("\n" +
                indent + "    public int get%s(final byte[] dst, final int dstOffset)\n" +
                indent + "    {\n" +
                indent + "        final int length = %d;\n" +
                indent + "        if (dstOffset < 0 || dstOffset > (dst.length - length))\n" +
                indent + "        {\n" +
                indent + "            throw new IndexOutOfBoundsException(" +
                "\"Copy will go out of range: offset=\" + dstOffset);\n" +
                indent + "        }\n\n" +
                "%s" +
                "%s" +
                indent + "        buffer.getBytes(offset + %d, dst, dstOffset, length);\n\n" +
                indent + "        return length;\n" +
                indent + "    }\n",
                Generators.toUpperFirstChar(propertyName),
                fieldLength,
                generateArrayFieldNotPresentCondition(inComposite, propertyToken.version(), indent),
                accessOrderListenerCall,
                offset);

            new Formatter(sb).format("\n" +
                indent + "    public String %s()\n" +
                indent + "    {\n" +
                "%s" +
                "%s" +
                indent + "        final byte[] dst = new byte[%d];\n" +
                indent + "        buffer.getBytes(offset + %d, dst, 0, %d);\n\n" +
                indent + "        int end = 0;\n" +
                indent + "        for (; end < %d && dst[end] != 0; ++end);\n\n" +
                indent + "        return new String(dst, 0, end, %s);\n" +
                indent + "    }\n\n",
                propertyName,
                generateStringNotPresentCondition(inComposite, propertyToken.version(), indent),
                accessOrderListenerCall,
                fieldLength,
                offset,
                fieldLength,
                fieldLength,
                charset(encoding.characterEncoding()));

            if (isAsciiEncoding(encoding.characterEncoding()))
            {
                new Formatter(sb).format("\n" +
                    indent + "    public int get%1$s(final Appendable value)\n" +
                    indent + "    {\n" +
                    "%2$s" +
                    "%5$s" +
                    indent + "        for (int i = 0; i < %3$d; ++i)\n" +
                    indent + "        {\n" +
                    indent + "            final int c = buffer.getByte(offset + %4$d + i) & 0xFF;\n" +
                    indent + "            if (c == 0)\n" +
                    indent + "            {\n" +
                    indent + "                return i;\n" +
                    indent + "            }\n\n" +
                    indent + "            try\n" +
                    indent + "            {\n" +
                    indent + "                value.append(c > 127 ? '?' : (char)c);\n" +
                    indent + "            }\n" +
                    indent + "            catch (final java.io.IOException ex)\n" +
                    indent + "            {\n" +
                    indent + "                throw new java.io.UncheckedIOException(ex);\n" +
                    indent + "            }\n" +
                    indent + "        }\n\n" +
                    indent + "        return %3$d;\n" +
                    indent + "    }\n\n",
                    Generators.toUpperFirstChar(propertyName),
                    generateStringNotPresentConditionForAppendable(inComposite, propertyToken.version(), indent),
                    fieldLength,
                    offset,
                    accessOrderListenerCall);
            }
        }
        else if (encoding.primitiveType() == PrimitiveType.UINT8)
        {
            new Formatter(sb).format("\n" +
                indent + "    public int get%s(final byte[] dst, final int dstOffset, final int length)\n" +
                indent + "    {\n" +
                "%s" +
                "%s" +
                indent + "        final int bytesCopied = Math.min(length, %d);\n" +
                indent + "        buffer.getBytes(offset + %d, dst, dstOffset, bytesCopied);\n\n" +
                indent + "        return bytesCopied;\n" +
                indent + "    }\n",
                Generators.toUpperFirstChar(propertyName),
                generateArrayFieldNotPresentCondition(inComposite, propertyToken.version(), indent),
                accessOrderListenerCall,
                fieldLength,
                offset);

            new Formatter(sb).format("\n" +
                indent + "    public int get%s(final %s dst, final int dstOffset, final int length)\n" +
                indent + "    {\n" +
                "%s" +
                "%s" +
                indent + "        final int bytesCopied = Math.min(length, %d);\n" +
                indent + "        buffer.getBytes(offset + %d, dst, dstOffset, bytesCopied);\n\n" +
                indent + "        return bytesCopied;\n" +
                indent + "    }\n",
                Generators.toUpperFirstChar(propertyName),
                fqMutableBuffer,
                generateArrayFieldNotPresentCondition(inComposite, propertyToken.version(), indent),
                accessOrderListenerCall,
                fieldLength,
                offset);

            new Formatter(sb).format("\n" +
                indent + "    public void wrap%s(final %s wrapBuffer)\n" +
                indent + "    {\n" +
                "%s" +
                "%s" +
                indent + "        wrapBuffer.wrap(buffer, offset + %d, %d);\n" +
                indent + "    }\n",
                Generators.toUpperFirstChar(propertyName),
                readOnlyBuffer,
                generateWrapFieldNotPresentCondition(propertyToken.version(), indent),
                accessOrderListenerCall,
                offset,
                fieldLength);
        }

        return sb;
    }

    private static void generateArrayLengthMethod(
        final String propertyName, final String indent, final int fieldLength, final StringBuilder sb)
    {
        final String formatPropertyName = formatPropertyName(propertyName);
        sb.append("\n")
            .append(indent).append("    public static int ").append(formatPropertyName).append("Length()\n")
            .append(indent).append("    {\n")
            .append(indent).append("        return ").append(fieldLength).append(";\n")
            .append(indent).append("    }\n\n");
    }

    private String byteOrderString(final Encoding encoding)
    {
        return sizeOfPrimitive(encoding) == 1 ? "" : ", java.nio.ByteOrder." + encoding.byteOrder();
    }

    private CharSequence generatePrimitiveArrayPropertyEncode(
        final String containingClassName,
        final String propertyName,
        final CharSequence accessOrderListenerCall,
        final Token typeToken,
        final String indent)
    {
        final Encoding encoding = typeToken.encoding();
        final PrimitiveType primitiveType = encoding.primitiveType();
        final String javaTypeName = javaTypeName(primitiveType);
        final int offset = typeToken.offset();
        final String byteOrderStr = byteOrderString(encoding);
        final int arrayLength = typeToken.arrayLength();
        final int typeSize = sizeOfPrimitive(encoding);

        final StringBuilder sb = new StringBuilder();
        final String className = formatClassName(containingClassName);

        generateArrayLengthMethod(propertyName, indent, arrayLength, sb);

        new Formatter(sb).format("\n" +
            indent + "    public %s %s(final int index, final %s value)\n" +
            indent + "    {\n" +
            indent + "        if (index < 0 || index >= %d)\n" +
            indent + "        {\n" +
            indent + "            throw new IndexOutOfBoundsException(\"index out of range: index=\" + index);\n" +
            indent + "        }\n\n" +
            "%s" +
            indent + "        final int pos = offset + %d + (index * %d);\n" +
            indent + "        %s;\n\n" +
            indent + "        return this;\n" +
            indent + "    }\n",
            className,
            propertyName,
            javaTypeName,
            arrayLength,
            accessOrderListenerCall,
            offset,
            typeSize,
            generatePut(primitiveType, "pos", "value", byteOrderStr));

        if (arrayLength > 1 && arrayLength <= 4)
        {
            sb.append(indent)
                .append("    public ")
                .append(className)
                .append(" put").append(Generators.toUpperFirstChar(propertyName))
                .append("(final ").append(javaTypeName).append(" value0");

            for (int i = 1; i < arrayLength; i++)
            {
                sb.append(", final ").append(javaTypeName).append(" value").append(i);
            }

            sb.append(")\n");
            sb.append(indent).append("    {\n");

            sb.append(accessOrderListenerCall);

            for (int i = 0; i < arrayLength; i++)
            {
                final String indexStr = "offset + " + (offset + (typeSize * i));

                sb.append(indent).append("        ")
                    .append(generatePut(primitiveType, indexStr, "value" + i, byteOrderStr))
                    .append(";\n");
            }

            sb.append("\n");
            sb.append(indent).append("        return this;\n");
            sb.append(indent).append("    }\n");
        }

        if (primitiveType == PrimitiveType.CHAR)
        {
            generateCharArrayEncodeMethods(
                containingClassName,
                propertyName,
                indent,
                accessOrderListenerCall,
                encoding,
                offset,
                arrayLength,
                sb);
        }
        else if (primitiveType == PrimitiveType.UINT8)
        {
            generateByteArrayEncodeMethods(
                containingClassName,
                propertyName,
                indent,
                accessOrderListenerCall,
                offset,
                arrayLength,
                sb);
        }

        return sb;
    }

    private void generateCharArrayEncodeMethods(
        final String containingClassName,
        final String propertyName,
        final String indent,
        final CharSequence accessOrderListenerCall,
        final Encoding encoding,
        final int offset,
        final int fieldLength,
        final StringBuilder sb)
    {
        generateCharacterEncodingMethod(sb, propertyName, encoding.characterEncoding(), indent);

        new Formatter(sb).format("\n" +
            indent + "    public %s put%s(final byte[] src, final int srcOffset)\n" +
            indent + "    {\n" +
            indent + "        final int length = %d;\n" +
            indent + "        if (srcOffset < 0 || srcOffset > (src.length - length))\n" +
            indent + "        {\n" +
            indent + "            throw new IndexOutOfBoundsException(" +
            "\"Copy will go out of range: offset=\" + srcOffset);\n" +
            indent + "        }\n\n" +
            "%s" +
            indent + "        buffer.putBytes(offset + %d, src, srcOffset, length);\n\n" +
            indent + "        return this;\n" +
            indent + "    }\n",
            formatClassName(containingClassName),
            Generators.toUpperFirstChar(propertyName),
            fieldLength,
            accessOrderListenerCall,
            offset);

        if (isAsciiEncoding(encoding.characterEncoding()))
        {
            new Formatter(sb).format("\n" +
                indent + "    public %1$s %2$s(final String src)\n" +
                indent + "    {\n" +
                indent + "        final int length = %3$d;\n" +
                indent + "        final int srcLength = null == src ? 0 : src.length();\n" +
                indent + "        if (srcLength > length)\n" +
                indent + "        {\n" +
                indent + "            throw new IndexOutOfBoundsException(" +
                "\"String too large for copy: byte length=\" + srcLength);\n" +
                indent + "        }\n\n" +
                "%5$s" +
                indent + "        buffer.putStringWithoutLengthAscii(offset + %4$d, src);\n\n" +
                indent + "        for (int start = srcLength; start < length; ++start)\n" +
                indent + "        {\n" +
                indent + "            buffer.putByte(offset + %4$d + start, (byte)0);\n" +
                indent + "        }\n\n" +
                indent + "        return this;\n" +
                indent + "    }\n",
                formatClassName(containingClassName),
                propertyName,
                fieldLength,
                offset,
                accessOrderListenerCall);

            new Formatter(sb).format("\n" +
                indent + "    public %1$s %2$s(final CharSequence src)\n" +
                indent + "    {\n" +
                indent + "        final int length = %3$d;\n" +
                indent + "        final int srcLength = null == src ? 0 : src.length();\n" +
                indent + "        if (srcLength > length)\n" +
                indent + "        {\n" +
                indent + "            throw new IndexOutOfBoundsException(" +
                "\"CharSequence too large for copy: byte length=\" + srcLength);\n" +
                indent + "        }\n\n" +
                "%5$s" +
                indent + "        buffer.putStringWithoutLengthAscii(offset + %4$d, src);\n\n" +
                indent + "        for (int start = srcLength; start < length; ++start)\n" +
                indent + "        {\n" +
                indent + "            buffer.putByte(offset + %4$d + start, (byte)0);\n" +
                indent + "        }\n\n" +
                indent + "        return this;\n" +
                indent + "    }\n",
                formatClassName(containingClassName),
                propertyName,
                fieldLength,
                offset,
                accessOrderListenerCall);
        }
        else
        {
            new Formatter(sb).format("\n" +
                indent + "    public %s %s(final String src)\n" +
                indent + "    {\n" +
                indent + "        final int length = %d;\n" +
                indent + "        final byte[] bytes = (null == src || src.isEmpty()) ?" +
                " org.agrona.collections.ArrayUtil.EMPTY_BYTE_ARRAY : src.getBytes(%s);\n" +
                indent + "        if (bytes.length > length)\n" +
                indent + "        {\n" +
                indent + "            throw new IndexOutOfBoundsException(" +
                "\"String too large for copy: byte length=\" + bytes.length);\n" +
                indent + "        }\n\n" +
                "%s" +
                indent + "        buffer.putBytes(offset + %d, bytes, 0, bytes.length);\n\n" +
                indent + "        for (int start = bytes.length; start < length; ++start)\n" +
                indent + "        {\n" +
                indent + "            buffer.putByte(offset + %d + start, (byte)0);\n" +
                indent + "        }\n\n" +
                indent + "        return this;\n" +
                indent + "    }\n",
                formatClassName(containingClassName),
                propertyName,
                fieldLength,
                charset(encoding.characterEncoding()),
                accessOrderListenerCall,
                offset,
                offset);
        }
    }

    private void generateByteArrayEncodeMethods(
        final String containingClassName,
        final String propertyName,
        final String indent,
        final CharSequence accessOrderListenerCall,
        final int offset,
        final int fieldLength,
        final StringBuilder sb)
    {
        new Formatter(sb).format("\n" +
            indent + "    public %s put%s(final byte[] src, final int srcOffset, final int length)\n" +
            indent + "    {\n" +
            indent + "        if (length > %d)\n" +
            indent + "        {\n" +
            indent + "            throw new IllegalStateException(" +
            "\"length > maxValue for type: \" + length);\n" +
            indent + "        }\n\n" +
            "%s" +
            indent + "        buffer.putBytes(offset + %d, src, srcOffset, length);\n" +
            indent + "        for (int i = length; i < %d; i++)\n" +
            indent + "        {\n" +
            indent + "            buffer.putByte(offset + %d + i, (byte)0);\n" +
            indent + "        }\n\n" +
            indent + "        return this;\n" +
            indent + "    }\n",
            formatClassName(containingClassName),
            Generators.toUpperFirstChar(propertyName),
            fieldLength,
            accessOrderListenerCall,
            offset,
            fieldLength,
            offset);

        new Formatter(sb).format("\n" +
            indent + "    public %s put%s(final %s src, final int srcOffset, final int length)\n" +
            indent + "    {\n" +
            indent + "        if (length > %d)\n" +
            indent + "        {\n" +
            indent + "            throw new IllegalStateException(" +
            "\"length > maxValue for type: \" + length);\n" +
            indent + "        }\n\n" +
            "%s" +
            indent + "        buffer.putBytes(offset + %d, src, srcOffset, length);\n" +
            indent + "        for (int i = length; i < %d; i++)\n" +
            indent + "        {\n" +
            indent + "            buffer.putByte(offset + %d + i, (byte)0);\n" +
            indent + "        }\n\n" +
            indent + "        return this;\n" +
            indent + "    }\n",
            formatClassName(containingClassName),
            Generators.toUpperFirstChar(propertyName),
            fqReadOnlyBuffer,
            fieldLength,
            accessOrderListenerCall,
            offset,
            fieldLength,
            offset);
    }

    private static int sizeOfPrimitive(final Encoding encoding)
    {
        return encoding.primitiveType().size();
    }

    private static void generateCharacterEncodingMethod(
        final StringBuilder sb, final String propertyName, final String characterEncoding, final String indent)
    {
        if (null != characterEncoding)
        {
            final String propName = formatPropertyName(propertyName);
            sb.append("\n")
                .append(indent).append("    public static String ").append(propName).append("CharacterEncoding()\n")
                .append(indent).append("    {\n")
                .append(indent).append("        return ").append(charsetName(characterEncoding)).append(";\n")
                .append(indent).append("    }\n");
        }
    }

    private void generateConstPropertyMethods(
        final StringBuilder sb, final String propertyName, final Token token, final String indent)
    {
        final String formattedPropertyName = formatPropertyName(propertyName);
        final Encoding encoding = token.encoding();
        if (encoding.primitiveType() != PrimitiveType.CHAR)
        {
            new Formatter(sb).format("\n" +
                indent + "    public %s %s()\n" +
                indent + "    {\n" +
                indent + "        return %s;\n" +
                indent + "    }\n",
                javaTypeName(encoding.primitiveType()),
                formattedPropertyName,
                generateLiteral(encoding.primitiveType(), encoding.constValue().toString()));

            return;
        }

        final String javaTypeName = javaTypeName(encoding.primitiveType());
        final byte[] constBytes = encoding.constValue().byteArrayValue(encoding.primitiveType());
        final CharSequence values = generateByteLiteralList(
            encoding.constValue().byteArrayValue(encoding.primitiveType()));

        new Formatter(sb).format("\n" +
            "\n" +
            indent + "    private static final byte[] %s_VALUE = { %s };\n",
            propertyName.toUpperCase(),
            values);

        generateArrayLengthMethod(formattedPropertyName, indent, constBytes.length, sb);

        new Formatter(sb).format("\n" +
            indent + "    public %s %s(final int index)\n" +
            indent + "    {\n" +
            indent + "        return %s_VALUE[index];\n" +
            indent + "    }\n\n",
            javaTypeName,
            formattedPropertyName,
            propertyName.toUpperCase());

        sb.append(String.format(
            indent + "    public int get%s(final byte[] dst, final int offset, final int length)\n" +
            indent + "    {\n" +
            indent + "        final int bytesCopied = Math.min(length, %d);\n" +
            indent + "        System.arraycopy(%s_VALUE, 0, dst, offset, bytesCopied);\n\n" +
            indent + "        return bytesCopied;\n" +
            indent + "    }\n",
            Generators.toUpperFirstChar(propertyName),
            constBytes.length,
            propertyName.toUpperCase()));

        if (constBytes.length > 1)
        {
            new Formatter(sb).format("\n" +
                indent + "    public String %s()\n" +
                indent + "    {\n" +
                indent + "        return \"%s\";\n" +
                indent + "    }\n\n",
                formattedPropertyName,
                encoding.constValue());
        }
        else
        {
            new Formatter(sb).format("\n" +
                indent + "    public byte %s()\n" +
                indent + "    {\n" +
                indent + "        return (byte)%s;\n" +
                indent + "    }\n\n",
                formattedPropertyName,
                encoding.constValue());
        }
    }

    private static CharSequence generateByteLiteralList(final byte[] bytes)
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

    private CharSequence generateFixedFlyweightCode(
        final String className, final int size, final String bufferImplementation)
    {
        final String schemaIdType = javaTypeName(ir.headerStructure().schemaIdType());
        final String schemaIdAccessorType = shouldGenerateInterfaces ? "int" : schemaIdType;
        final String schemaVersionType = javaTypeName(ir.headerStructure().schemaVersionType());
        final String schemaVersionAccessorType = shouldGenerateInterfaces ? "int" : schemaVersionType;
        final String semanticVersion = ir.semanticVersion() == null ? "" : ir.semanticVersion();

        return String.format(
            "    public static final %5$s SCHEMA_ID = %6$s;\n" +
            "    public static final %7$s SCHEMA_VERSION = %8$s;\n" +
            "    public static final String SEMANTIC_VERSION = \"%11$s\";\n" +
            "    public static final int ENCODED_LENGTH = %2$d;\n" +
            "    public static final java.nio.ByteOrder BYTE_ORDER = java.nio.ByteOrder.%4$s;\n\n" +
            "    private int offset;\n" +
            "    private %3$s buffer;\n\n" +
            "    public %1$s wrap(final %3$s buffer, final int offset)\n" +
            "    {\n" +
            "        if (buffer != this.buffer)\n" +
            "        {\n" +
            "            this.buffer = buffer;\n" +
            "        }\n" +
            "        this.offset = offset;\n\n" +
            "        return this;\n" +
            "    }\n\n" +
            "    public %3$s buffer()\n" +
            "    {\n" +
            "        return buffer;\n" +
            "    }\n\n" +
            "    public int offset()\n" +
            "    {\n" +
            "        return offset;\n" +
            "    }\n\n" +
            "    public int encodedLength()\n" +
            "    {\n" +
            "        return ENCODED_LENGTH;\n" +
            "    }\n\n" +
            "    public %9$s sbeSchemaId()\n" +
            "    {\n" +
            "        return SCHEMA_ID;\n" +
            "    }\n\n" +
            "    public %10$s sbeSchemaVersion()\n" +
            "    {\n" +
            "        return SCHEMA_VERSION;\n" +
            "    }\n",
            className,
            size,
            bufferImplementation,
            ir.byteOrder(),
            schemaIdType,
            generateLiteral(ir.headerStructure().schemaIdType(), Integer.toString(ir.id())),
            schemaVersionType,
            generateLiteral(ir.headerStructure().schemaVersionType(), Integer.toString(ir.version())),
            schemaIdAccessorType,
            schemaVersionAccessorType,
            semanticVersion);
    }

    private CharSequence generateDecoderFlyweightCode(
        final FieldPrecedenceModel fieldPrecedenceModel,
        final String className,
        final Token token)
    {
        final String headerClassName = formatClassName(ir.headerStructure().tokens().get(0).applicableTypeName());

        final StringBuilder methods = new StringBuilder();

        methods.append(generateDecoderWrapListener(fieldPrecedenceModel, "    "));

        methods.append("    public ").append(className).append(" wrap(\n")
            .append("        final ").append(readOnlyBuffer).append(" buffer,\n")
            .append("        final int offset,\n")
            .append("        final int actingBlockLength,\n")
            .append("        final int actingVersion)\n")
            .append("    {\n")
            .append("        if (buffer != this.buffer)\n")
            .append("        {\n")
            .append("            this.buffer = buffer;\n")
            .append("        }\n")
            .append("        this.offset = offset;\n")
            .append("        this.actingBlockLength = actingBlockLength;\n")
            .append("        this.actingVersion = actingVersion;\n")
            .append("        limit(offset + actingBlockLength);\n\n")
            .append(generateAccessOrderListenerCall(fieldPrecedenceModel, "        ", "onWrap", "actingVersion"))
            .append("        return this;\n")
            .append("    }\n\n");

        methods.append("    public ").append(className).append(" wrapAndApplyHeader(\n")
            .append("        final ").append(readOnlyBuffer).append(" buffer,\n")
            .append("        final int offset,\n")
            .append("        final ").append(headerClassName).append("Decoder headerDecoder)\n")
            .append("    {\n")
            .append("        headerDecoder.wrap(buffer, offset);\n\n")
            .append("        final int templateId = headerDecoder.templateId();\n")
            .append("        if (TEMPLATE_ID != templateId)\n")
            .append("        {\n")
            .append("            throw new IllegalStateException(\"Invalid TEMPLATE_ID: \" + templateId);\n")
            .append("        }\n\n")
            .append("        return wrap(\n")
            .append("            buffer,\n")
            .append("            offset + ").append(headerClassName).append("Decoder.ENCODED_LENGTH,\n")
            .append("            headerDecoder.blockLength(),\n")
            .append("            headerDecoder.version());\n")
            .append("    }\n\n");

        methods.append("    public ").append(className).append(" sbeRewind()\n")
            .append("    {\n")
            .append("        return wrap(buffer, offset, actingBlockLength, actingVersion);\n")
            .append("    }\n\n");

        methods.append("    public int sbeDecodedLength()\n")
            .append("    {\n")
            .append("        final int currentLimit = limit();\n");

        if (null != fieldPrecedenceModel)
        {
            methods.append("        final int currentCodecState = codecState();\n");
        }

        methods
            .append("        sbeSkip();\n")
            .append("        final int decodedLength = encodedLength();\n")
            .append("        limit(currentLimit);\n\n");

        if (null != fieldPrecedenceModel)
        {
            methods.append("        if (").append(precedenceChecksFlagName).append(")\n")
                .append("        {\n")
                .append("            codecState(currentCodecState);\n")
                .append("        }\n\n");
        }

        methods.append("        return decodedLength;\n")
            .append("    }\n\n");

        methods.append("    public int actingVersion()\n")
            .append("    {\n")
            .append("        return actingVersion;\n")
            .append("    }\n\n");

        return generateFlyweightCode(DECODER, className, token, methods.toString(), readOnlyBuffer);
    }

    private CharSequence generateFlyweightCode(
        final CodecType codecType,
        final String className,
        final Token token,
        final String wrapMethod,
        final String bufferImplementation)
    {
        final HeaderStructure headerStructure = ir.headerStructure();
        final String blockLengthType = javaTypeName(headerStructure.blockLengthType());
        final String blockLengthAccessorType = shouldGenerateInterfaces ? "int" : blockLengthType;
        final String templateIdType = javaTypeName(headerStructure.templateIdType());
        final String templateIdAccessorType = shouldGenerateInterfaces ? "int" : templateIdType;
        final String schemaIdType = javaTypeName(headerStructure.schemaIdType());
        final String schemaIdAccessorType = shouldGenerateInterfaces ? "int" : schemaIdType;
        final String schemaVersionType = javaTypeName(headerStructure.schemaVersionType());
        final String schemaVersionAccessorType = shouldGenerateInterfaces ? "int" : schemaVersionType;
        final String semanticType = token.encoding().semanticType() == null ? "" : token.encoding().semanticType();
        final String semanticVersion = ir.semanticVersion() == null ? "" : ir.semanticVersion();
        final String actingFields = codecType == CodecType.ENCODER ?
            "" :
            "    int actingBlockLength;\n" +
            "    int actingVersion;\n";

        return String.format(
            "    public static final %1$s BLOCK_LENGTH = %2$s;\n" +
            "    public static final %3$s TEMPLATE_ID = %4$s;\n" +
            "    public static final %5$s SCHEMA_ID = %6$s;\n" +
            "    public static final %7$s SCHEMA_VERSION = %8$s;\n" +
            "    public static final String SEMANTIC_VERSION = \"%19$s\";\n" +
            "    public static final java.nio.ByteOrder BYTE_ORDER = java.nio.ByteOrder.%14$s;\n\n" +
            "    private final %9$s parentMessage = this;\n" +
            "    private %11$s buffer;\n" +
            "    private int offset;\n" +
            "    private int limit;\n" +
            "%13$s" +
            "\n" +
            "    public %15$s sbeBlockLength()\n" +
            "    {\n" +
            "        return BLOCK_LENGTH;\n" +
            "    }\n\n" +
            "    public %16$s sbeTemplateId()\n" +
            "    {\n" +
            "        return TEMPLATE_ID;\n" +
            "    }\n\n" +
            "    public %17$s sbeSchemaId()\n" +
            "    {\n" +
            "        return SCHEMA_ID;\n" +
            "    }\n\n" +
            "    public %18$s sbeSchemaVersion()\n" +
            "    {\n" +
            "        return SCHEMA_VERSION;\n" +
            "    }\n\n" +
            "    public String sbeSemanticType()\n" +
            "    {\n" +
            "        return \"%10$s\";\n" +
            "    }\n\n" +
            "    public %11$s buffer()\n" +
            "    {\n" +
            "        return buffer;\n" +
            "    }\n\n" +
            "    public int offset()\n" +
            "    {\n" +
            "        return offset;\n" +
            "    }\n\n" +
            "%12$s" +
            "    public int encodedLength()\n" +
            "    {\n" +
            "        return limit - offset;\n" +
            "    }\n\n" +
            "    public int limit()\n" +
            "    {\n" +
            "        return limit;\n" +
            "    }\n\n" +
            "    public void limit(final int limit)\n" +
            "    {\n" +
            "        this.limit = limit;\n" +
            "    }\n",
            blockLengthType,
            generateLiteral(headerStructure.blockLengthType(), Integer.toString(token.encodedLength())),
            templateIdType,
            generateLiteral(headerStructure.templateIdType(), Integer.toString(token.id())),
            schemaIdType,
            generateLiteral(headerStructure.schemaIdType(), Integer.toString(ir.id())),
            schemaVersionType,
            generateLiteral(headerStructure.schemaVersionType(), Integer.toString(ir.version())),
            className,
            semanticType,
            bufferImplementation,
            wrapMethod,
            actingFields,
            ir.byteOrder(),
            blockLengthAccessorType,
            templateIdAccessorType,
            schemaIdAccessorType,
            schemaVersionAccessorType,
            semanticVersion);
    }

    private CharSequence generateEncoderFlyweightCode(
        final String className,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final Token token)
    {
        final String wrapMethod =
            "    public " + className + " wrap(final " + mutableBuffer + " buffer, final int offset)\n" +
            "    {\n" +
            "        if (buffer != this.buffer)\n" +
            "        {\n" +
            "            this.buffer = buffer;\n" +
            "        }\n" +
            "        this.offset = offset;\n" +
            "        limit(offset + BLOCK_LENGTH);\n\n" +
            generateEncoderWrapListener(fieldPrecedenceModel, "        ") +
            "        return this;\n" +
            "    }\n\n";

        final StringBuilder builder = new StringBuilder(
            "    public %1$s wrapAndApplyHeader(\n" +
            "        final %2$s buffer, final int offset, final %3$s headerEncoder)\n" +
            "    {\n" +
            "        headerEncoder\n" +
            "            .wrap(buffer, offset)");

        for (final Token headerToken : ir.headerStructure().tokens())
        {
            if (!headerToken.isConstantEncoding())
            {
                switch (headerToken.name())
                {
                    case "blockLength":
                        builder.append("\n            .blockLength(BLOCK_LENGTH)");
                        break;

                    case "templateId":
                        builder.append("\n            .templateId(TEMPLATE_ID)");
                        break;

                    case "schemaId":
                        builder.append("\n            .schemaId(SCHEMA_ID)");
                        break;

                    case "version":
                        builder.append("\n            .version(SCHEMA_VERSION)");
                        break;
                }
            }
        }

        builder.append(";\n\n        return wrap(buffer, offset + %3$s.ENCODED_LENGTH);\n" + "    }\n\n");

        final String wrapAndApplyMethod = String.format(
            builder.toString(),
            className,
            mutableBuffer,
            formatClassName(ir.headerStructure().tokens().get(0).applicableTypeName() + "Encoder"));

        return generateFlyweightCode(
            CodecType.ENCODER, className, token, wrapMethod + wrapAndApplyMethod, mutableBuffer);
    }

    private void generateEncoderFields(
        final StringBuilder sb,
        final String containingClassName,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final List<Token> tokens,
        final String indent)
    {
        Generators.forEachField(
            tokens,
            (fieldToken, typeToken) ->
            {
                final String propertyName = formatPropertyName(fieldToken.name());
                final String typeName = encoderName(typeToken.name());

                generateFieldIdMethod(sb, fieldToken, indent);
                generateFieldSinceVersionMethod(sb, fieldToken, indent);
                generateEncodingOffsetMethod(sb, propertyName, fieldToken.offset(), indent);
                generateEncodingLengthMethod(sb, propertyName, typeToken.encodedLength(), indent);
                generateFieldMetaAttributeMethod(sb, fieldToken, indent);
                generateAccessOrderListenerMethod(sb, fieldPrecedenceModel, indent + "    ", fieldToken);
                final CharSequence accessOrderListenerCall = generateAccessOrderListenerCall(
                    fieldPrecedenceModel, indent + "        ", fieldToken);

                switch (typeToken.signal())
                {
                    case ENCODING:
                        generatePrimitiveEncoder(sb, containingClassName, propertyName,
                            accessOrderListenerCall, typeToken, indent);
                        break;

                    case BEGIN_ENUM:
                        generateEnumEncoder(sb, containingClassName,
                            accessOrderListenerCall, fieldToken, propertyName, typeToken, indent);
                        break;

                    case BEGIN_SET:
                        generateBitSetProperty(
                            sb, false, ENCODER, propertyName, accessOrderListenerCall,
                            fieldToken, typeToken, indent, typeName);
                        break;

                    case BEGIN_COMPOSITE:
                        generateCompositeProperty(
                            sb, false, ENCODER, propertyName, accessOrderListenerCall,
                            fieldToken, typeToken, indent, typeName);
                        break;

                    default:
                        break;
                }
            });
    }

    private void generateDecoderFields(
        final StringBuilder sb,
        final FieldPrecedenceModel fieldPrecedenceModel,
        final List<Token> tokens,
        final String indent)
    {
        Generators.forEachField(
            tokens,
            (fieldToken, typeToken) ->
            {
                final String propertyName = formatPropertyName(fieldToken.name());
                final String typeName = decoderName(typeToken.name());

                generateFieldIdMethod(sb, fieldToken, indent);
                generateFieldSinceVersionMethod(sb, fieldToken, indent);
                generateEncodingOffsetMethod(sb, propertyName, fieldToken.offset(), indent);
                generateEncodingLengthMethod(sb, propertyName, typeToken.encodedLength(), indent);
                generateFieldMetaAttributeMethod(sb, fieldToken, indent);

                generateAccessOrderListenerMethod(sb, fieldPrecedenceModel, indent + "    ", fieldToken);
                final CharSequence accessOrderListenerCall = generateAccessOrderListenerCall(
                    fieldPrecedenceModel, indent + "        ", fieldToken);

                switch (typeToken.signal())
                {
                    case ENCODING:
                        generatePrimitiveDecoder(
                            sb, false, propertyName, accessOrderListenerCall, fieldToken, typeToken, indent);
                        break;

                    case BEGIN_ENUM:
                        generateEnumDecoder(
                            sb, false, accessOrderListenerCall, fieldToken, propertyName, typeToken, indent);
                        break;

                    case BEGIN_SET:
                        generateBitSetProperty(
                            sb, false, DECODER, propertyName, accessOrderListenerCall,
                            fieldToken, typeToken, indent, typeName);
                        break;

                    case BEGIN_COMPOSITE:
                        generateCompositeProperty(
                            sb, false, DECODER, propertyName, accessOrderListenerCall,
                            fieldToken, typeToken, indent, typeName);
                        break;

                    default:
                        break;
                }
            });
    }

    private static void generateFieldIdMethod(final StringBuilder sb, final Token token, final String indent)
    {
        final String propertyName = formatPropertyName(token.name());
        sb.append("\n")
            .append(indent).append("    public static int ").append(propertyName).append("Id()\n")
            .append(indent).append("    {\n")
            .append(indent).append("        return ").append(token.id()).append(";\n")
            .append(indent).append("    }\n");
    }

    private static void generateEncodingOffsetMethod(
        final StringBuilder sb, final String name, final int offset, final String indent)
    {
        final String propertyName = formatPropertyName(name);
        sb.append("\n")
            .append(indent).append("    public static int ").append(propertyName).append("EncodingOffset()\n")
            .append(indent).append("    {\n")
            .append(indent).append("        return ").append(offset).append(";\n")
            .append(indent).append("    }\n");
    }

    private static void generateEncodingLengthMethod(
        final StringBuilder sb, final String name, final int length, final String indent)
    {
        final String propertyName = formatPropertyName(name);
        sb.append("\n")
            .append(indent).append("    public static int ").append(propertyName).append("EncodingLength()\n")
            .append(indent).append("    {\n")
            .append(indent).append("        return ").append(length).append(";\n")
            .append(indent).append("    }\n");
    }

    private static void generateFieldSinceVersionMethod(final StringBuilder sb, final Token token, final String indent)
    {
        final String propertyName = formatPropertyName(token.name());
        sb.append("\n")
            .append(indent).append("    public static int ").append(propertyName).append("SinceVersion()\n")
            .append(indent).append("    {\n")
            .append(indent).append("        return ").append(token.version()).append(";\n")
            .append(indent).append("    }\n");
    }

    private static void generateFieldMetaAttributeMethod(final StringBuilder sb, final Token token, final String indent)
    {
        final Encoding encoding = token.encoding();
        final String epoch = encoding.epoch() == null ? "" : encoding.epoch();
        final String timeUnit = encoding.timeUnit() == null ? "" : encoding.timeUnit();
        final String semanticType = encoding.semanticType() == null ? "" : encoding.semanticType();
        final String presence = encoding.presence().toString().toLowerCase();
        final String propertyName = formatPropertyName(token.name());

        sb.append("\n")
            .append(indent).append("    public static String ")
            .append(propertyName).append("MetaAttribute(final MetaAttribute metaAttribute)\n")
            .append(indent).append("    {\n")
            .append(indent).append("        if (MetaAttribute.PRESENCE == metaAttribute)\n")
            .append(indent).append("        {\n")
            .append(indent).append("            return \"").append(presence).append("\";\n")
            .append(indent).append("        }\n");

        if (!Strings.isEmpty(epoch))
        {
            sb.append(indent).append("        if (MetaAttribute.EPOCH == metaAttribute)\n")
                .append(indent).append("        {\n")
                .append(indent).append("            return \"").append(epoch).append("\";\n")
                .append(indent).append("        }\n");
        }

        if (!Strings.isEmpty(timeUnit))
        {
            sb.append(indent).append("        if (MetaAttribute.TIME_UNIT == metaAttribute)\n")
                .append(indent).append("        {\n")
                .append(indent).append("            return \"").append(timeUnit).append("\";\n")
                .append(indent).append("        }\n");
        }

        if (!Strings.isEmpty(semanticType))
        {
            sb.append(indent).append("        if (MetaAttribute.SEMANTIC_TYPE == metaAttribute)\n")
                .append(indent).append("        {\n")
                .append(indent).append("            return \"").append(semanticType).append("\";\n")
                .append(indent).append("        }\n");
        }

        sb.append("\n")
            .append(indent).append("        return \"\";\n")
            .append(indent).append("    }\n");
    }

    private void generateEnumDecoder(
        final StringBuilder sb,
        final boolean inComposite,
        final CharSequence accessOrderListenerCall,
        final Token fieldToken,
        final String propertyName,
        final Token typeToken,
        final String indent)
    {
        final String enumName = formatClassName(typeToken.applicableTypeName());
        final Encoding encoding = typeToken.encoding();
        final String javaTypeName = javaTypeName(encoding.primitiveType());

        if (fieldToken.isConstantEncoding())
        {
            final String enumValueStr = formatClassName(
                fieldToken.encoding().constValue().toString());

            new Formatter(sb).format(
                "\n" +
                indent + "    public %s %sRaw()\n" +
                indent + "    {\n" +
                indent + "        return %s.value();\n" +
                indent + "    }\n\n",
                javaTypeName,
                propertyName,
                enumValueStr);

            new Formatter(sb).format(
                "\n" +
                indent + "    public %s %s()\n" +
                indent + "    {\n" +
                indent + "        return %s;\n" +
                indent + "    }\n\n",
                enumName,
                propertyName,
                enumValueStr);
        }
        else
        {
            final String rawGetStr = generateGet(
                encoding.primitiveType(), "offset + " + typeToken.offset(), byteOrderString(encoding));

            new Formatter(sb).format(
                "\n" +
                indent + "    public %s %sRaw()\n" +
                indent + "    {\n" +
                "%s" +
                "%s" +
                indent + "        return %s;\n" +
                indent + "    }\n",
                javaTypeName,
                formatPropertyName(propertyName),
                generateFieldNotPresentCondition(inComposite, fieldToken.version(), encoding, indent),
                accessOrderListenerCall,
                rawGetStr);

            new Formatter(sb).format(
                "\n" +
                indent + "    public %s %s()\n" +
                indent + "    {\n" +
                "%s" +
                "%s" +
                indent + "        return %s.get(%s);\n" +
                indent + "    }\n\n",
                enumName,
                propertyName,
                generatePropertyNotPresentCondition(inComposite, DECODER, fieldToken, enumName, indent),
                accessOrderListenerCall,
                enumName,
                rawGetStr);
        }
    }

    private void generateEnumEncoder(
        final StringBuilder sb,
        final String containingClassName,
        final CharSequence accessOrderListenerCall,
        final Token fieldToken,
        final String propertyName,
        final Token typeToken,
        final String indent)
    {
        if (!fieldToken.isConstantEncoding())
        {
            final String enumName = formatClassName(typeToken.applicableTypeName());
            final Encoding encoding = typeToken.encoding();
            final int offset = typeToken.offset();
            final String byteOrderString = byteOrderString(encoding);

            new Formatter(sb).format("\n" +
                indent + "    public %s %s(final %s value)\n" +
                indent + "    {\n" +
                "%s" +
                indent + "        %s;\n" +
                indent + "        return this;\n" +
                indent + "    }\n",
                formatClassName(containingClassName),
                propertyName,
                enumName,
                accessOrderListenerCall,
                generatePut(encoding.primitiveType(), "offset + " + offset, "value.value()", byteOrderString));
        }
    }

    private void generateBitSetProperty(
        final StringBuilder sb,
        final boolean inComposite,
        final CodecType codecType,
        final String propertyName,
        final CharSequence accessOrderListenerCall,
        final Token propertyToken,
        final Token bitsetToken,
        final String indent,
        final String bitSetName)
    {
        new Formatter(sb).format("\n" +
            indent + "    private final %s %s = new %s();\n",
            bitSetName,
            propertyName,
            bitSetName);

        generateFlyweightPropertyJavadoc(sb, indent + INDENT, propertyToken, bitSetName);
        new Formatter(sb).format("\n" +
            indent + "    public %s %s()\n" +
            indent + "    {\n" +
            "%s" +
            "%s" +
            indent + "        %s.wrap(buffer, offset + %d);\n" +
            indent + "        return %s;\n" +
            indent + "    }\n",
            bitSetName,
            propertyName,
            generatePropertyNotPresentCondition(inComposite, codecType, propertyToken, null, indent),
            accessOrderListenerCall,
            propertyName,
            bitsetToken.offset(),
            propertyName);
    }

    private void generateCompositeProperty(
        final StringBuilder sb,
        final boolean inComposite,
        final CodecType codecType,
        final String propertyName,
        final CharSequence accessOrderListenerCall,
        final Token propertyToken,
        final Token compositeToken,
        final String indent,
        final String compositeName)
    {
        new Formatter(sb).format("\n" +
            indent + "    private final %s %s = new %s();\n",
            compositeName,
            propertyName,
            compositeName);

        generateFlyweightPropertyJavadoc(sb, indent + INDENT, propertyToken, compositeName);
        new Formatter(sb).format("\n" +
            indent + "    public %s %s()\n" +
            indent + "    {\n" +
            "%s" +
            "%s" +
            indent + "        %s.wrap(buffer, offset + %d);\n" +
            indent + "        return %s;\n" +
            indent + "    }\n",
            compositeName,
            propertyName,
            generatePropertyNotPresentCondition(inComposite, codecType, propertyToken, null, indent),
            accessOrderListenerCall,
            propertyName,
            compositeToken.offset(),
            propertyName);
    }

    private String generateGet(final PrimitiveType type, final String index, final String byteOrder)
    {
        switch (type)
        {
            case CHAR:
            case INT8:
                return "buffer.getByte(" + index + ")";

            case UINT8:
                return "((short)(buffer.getByte(" + index + ") & 0xFF))";

            case INT16:
                return "buffer.getShort(" + index + byteOrder + ")";

            case UINT16:
                return "(buffer.getShort(" + index + byteOrder + ") & 0xFFFF)";

            case INT32:
                return "buffer.getInt(" + index + byteOrder + ")";

            case UINT32:
                return "(buffer.getInt(" + index + byteOrder + ") & 0xFFFF_FFFFL)";

            case FLOAT:
                return "buffer.getFloat(" + index + byteOrder + ")";

            case INT64:
            case UINT64:
                return "buffer.getLong(" + index + byteOrder + ")";

            case DOUBLE:
                return "buffer.getDouble(" + index + byteOrder + ")";

            default:
                break;
        }

        throw new IllegalArgumentException("primitive type not supported: " + type);
    }

    private String generatePut(
        final PrimitiveType type, final String index, final String value, final String byteOrder)
    {
        switch (type)
        {
            case CHAR:
            case INT8:
                return "buffer.putByte(" + index + ", " + value + ")";

            case UINT8:
                return "buffer.putByte(" + index + ", (byte)" + value + ")";

            case INT16:
                return "buffer.putShort(" + index + ", " + value + byteOrder + ")";

            case UINT16:
                return "buffer.putShort(" + index + ", (short)" + value + byteOrder + ")";

            case INT32:
                return "buffer.putInt(" + index + ", " + value + byteOrder + ")";

            case UINT32:
                return "buffer.putInt(" + index + ", (int)" + value + byteOrder + ")";

            case FLOAT:
                return "buffer.putFloat(" + index + ", " + value + byteOrder + ")";

            case INT64:
            case UINT64:
                return "buffer.putLong(" + index + ", " + value + byteOrder + ")";

            case DOUBLE:
                return "buffer.putDouble(" + index + ", " + value + byteOrder + ")";

            default:
                break;
        }

        throw new IllegalArgumentException("primitive type not supported: " + type);
    }

    private String generateChoiceIsEmpty(final PrimitiveType type)
    {
        return "\n" +
            "    public boolean isEmpty()\n" +
            "    {\n" +
            "        return " + generateChoiceIsEmptyInner(type) + ";\n" +
            "    }\n";
    }

    private String generateChoiceIsEmptyInner(final PrimitiveType type)
    {
        switch (type)
        {
            case UINT8:
                return "0 == buffer.getByte(offset)";

            case UINT16:
                return "0 == buffer.getShort(offset)";

            case UINT32:
                return "0 == buffer.getInt(offset)";

            case UINT64:
                return "0 == buffer.getLong(offset)";

            default:
                break;
        }

        throw new IllegalArgumentException("primitive type not supported: " + type);
    }

    private String generateChoiceGet(final PrimitiveType type, final String bitIndex, final String byteOrder)
    {
        switch (type)
        {
            case UINT8:
                return "0 != (buffer.getByte(offset) & (1 << " + bitIndex + "))";

            case UINT16:
                return "0 != (buffer.getShort(offset" + byteOrder + ") & (1 << " + bitIndex + "))";

            case UINT32:
                return "0 != (buffer.getInt(offset" + byteOrder + ") & (1 << " + bitIndex + "))";

            case UINT64:
                return "0 != (buffer.getLong(offset" + byteOrder + ") & (1L << " + bitIndex + "))";

            default:
                break;
        }

        throw new IllegalArgumentException("primitive type not supported: " + type);
    }

    private String generateStaticChoiceGet(final PrimitiveType type, final String bitIndex)
    {
        switch (type)
        {
            case UINT8:
            case UINT16:
            case UINT32:
                return "0 != (value & (1 << " + bitIndex + "))";

            case UINT64:
                return "0 != (value & (1L << " + bitIndex + "))";

            default:
                break;
        }

        throw new IllegalArgumentException("primitive type not supported: " + type);
    }

    private String generateChoicePut(final PrimitiveType type, final String bitIdx, final String byteOrder)
    {
        switch (type)
        {
            case UINT8:
                return
                    "        byte bits = buffer.getByte(offset);\n" +
                    "        bits = (byte)(value ? bits | (1 << " + bitIdx + ") : bits & ~(1 << " + bitIdx + "));\n" +
                    "        buffer.putByte(offset, bits);";

            case UINT16:
                return
                    "        short bits = buffer.getShort(offset" + byteOrder + ");\n" +
                    "        bits = (short)(value ? bits | (1 << " + bitIdx + ") : bits & ~(1 << " + bitIdx + "));\n" +
                    "        buffer.putShort(offset, bits" + byteOrder + ");";

            case UINT32:
                return
                    "        int bits = buffer.getInt(offset" + byteOrder + ");\n" +
                    "        bits = value ? bits | (1 << " + bitIdx + ") : bits & ~(1 << " + bitIdx + ");\n" +
                    "        buffer.putInt(offset, bits" + byteOrder + ");";

            case UINT64:
                return
                    "        long bits = buffer.getLong(offset" + byteOrder + ");\n" +
                    "        bits = value ? bits | (1L << " + bitIdx + ") : bits & ~(1L << " + bitIdx + ");\n" +
                    "        buffer.putLong(offset, bits" + byteOrder + ");";

            default:
                break;
        }

        throw new IllegalArgumentException("primitive type not supported: " + type);
    }

    private String generateStaticChoicePut(final PrimitiveType type, final String bitIdx)
    {
        switch (type)
        {
            case UINT8:
                return
                    "        return (byte)(value ? bits | (1 << " + bitIdx + ") : bits & ~(1 << " + bitIdx + "));\n";

            case UINT16:
                return
                    "        return (short)(value ? bits | (1 << " + bitIdx + ") : bits & ~(1 << " + bitIdx + "));\n";

            case UINT32:
                return
                    "        return value ? bits | (1 << " + bitIdx + ") : bits & ~(1 << " + bitIdx + ");\n";

            case UINT64:
                return
                    "        return value ? bits | (1L << " + bitIdx + ") : bits & ~(1L << " + bitIdx + ");\n";

            default:
                break;
        }

        throw new IllegalArgumentException("primitive type not supported: " + type);
    }

    private void generateEncoderDisplay(final StringBuilder sb, final String decoderName)
    {
        appendToString(sb);

        sb.append('\n');
        append(sb, INDENT, "public StringBuilder appendTo(final StringBuilder builder)");
        append(sb, INDENT, "{");
        append(sb, INDENT, "    if (null == buffer)");
        append(sb, INDENT, "    {");
        append(sb, INDENT, "        return builder;");
        append(sb, INDENT, "    }");
        sb.append('\n');
        append(sb, INDENT, "    final " + decoderName + " decoder = new " + decoderName + "();");
        append(sb, INDENT, "    decoder.wrap(buffer, offset, BLOCK_LENGTH, SCHEMA_VERSION);");
        sb.append('\n');
        append(sb, INDENT, "    return decoder.appendTo(builder);");
        append(sb, INDENT, "}");
    }

    private CharSequence generateCompositeEncoderDisplay(final String decoderName)
    {
        final StringBuilder sb = new StringBuilder();

        appendToString(sb);
        sb.append('\n');
        append(sb, INDENT, "public StringBuilder appendTo(final StringBuilder builder)");
        append(sb, INDENT, "{");
        append(sb, INDENT, "    if (null == buffer)");
        append(sb, INDENT, "    {");
        append(sb, INDENT, "        return builder;");
        append(sb, INDENT, "    }");
        sb.append('\n');
        append(sb, INDENT, "    final " + decoderName + " decoder = new " + decoderName + "();");
        append(sb, INDENT, "    decoder.wrap(buffer, offset);");
        sb.append('\n');
        append(sb, INDENT, "    return decoder.appendTo(builder);");
        append(sb, INDENT, "}");

        return sb;
    }

    private CharSequence generateCompositeDecoderDisplay(final List<Token> tokens)
    {
        final StringBuilder sb = new StringBuilder();

        appendToString(sb);
        sb.append('\n');
        append(sb, INDENT, "public StringBuilder appendTo(final StringBuilder builder)");
        append(sb, INDENT, "{");
        append(sb, INDENT, "    if (null == buffer)");
        append(sb, INDENT, "    {");
        append(sb, INDENT, "        return builder;");
        append(sb, INDENT, "    }");
        sb.append('\n');
        Separator.BEGIN_COMPOSITE.appendToGeneratedBuilder(sb, INDENT + INDENT);

        int lengthBeforeLastGeneratedSeparator = -1;

        for (int i = 1, end = tokens.size() - 1; i < end;)
        {
            final Token encodingToken = tokens.get(i);
            final String propertyName = formatPropertyName(encodingToken.name());
            lengthBeforeLastGeneratedSeparator = writeTokenDisplay(propertyName, encodingToken, sb, INDENT + INDENT);
            i += encodingToken.componentTokenCount();
        }

        if (-1 != lengthBeforeLastGeneratedSeparator)
        {
            sb.setLength(lengthBeforeLastGeneratedSeparator);
        }

        Separator.END_COMPOSITE.appendToGeneratedBuilder(sb, INDENT + INDENT);
        sb.append('\n');
        append(sb, INDENT, "    return builder;");
        append(sb, INDENT, "}");

        return sb;
    }

    private CharSequence generateChoiceDisplay(final List<Token> tokens)
    {
        final StringBuilder sb = new StringBuilder();

        appendToString(sb);
        sb.append('\n');
        append(sb, INDENT, "public StringBuilder appendTo(final StringBuilder builder)");
        append(sb, INDENT, "{");
        Separator.BEGIN_SET.appendToGeneratedBuilder(sb, INDENT + INDENT);
        append(sb, INDENT, "    boolean atLeastOne = false;");

        for (final Token token : tokens)
        {
            if (token.signal() == Signal.CHOICE)
            {
                final String choiceName = formatPropertyName(token.name());
                append(sb, INDENT, "    if (" + choiceName + "())");
                append(sb, INDENT, "    {");
                append(sb, INDENT, "        if (atLeastOne)");
                append(sb, INDENT, "        {");
                Separator.ENTRY.appendToGeneratedBuilder(sb, INDENT + INDENT + INDENT + INDENT);
                append(sb, INDENT, "        }");
                append(sb, INDENT, "        builder.append(\"" + choiceName + "\");");
                append(sb, INDENT, "        atLeastOne = true;");
                append(sb, INDENT, "    }");
            }
        }

        Separator.END_SET.appendToGeneratedBuilder(sb, INDENT + INDENT);
        sb.append('\n');
        append(sb, INDENT, "    return builder;");
        append(sb, INDENT, "}");

        return sb;
    }

    private void generateDecoderDisplay(
        final StringBuilder sb,
        final String name,
        final List<Token> tokens,
        final List<Token> groups,
        final List<Token> varData)
    {
        appendMessageToString(sb, decoderName(name));
        sb.append('\n');
        append(sb, INDENT, "public StringBuilder appendTo(final StringBuilder builder)");
        append(sb, INDENT, "{");
        append(sb, INDENT, "    if (null == buffer)");
        append(sb, INDENT, "    {");
        append(sb, INDENT, "        return builder;");
        append(sb, INDENT, "    }");
        sb.append('\n');
        append(sb, INDENT, "    final int originalLimit = limit();");
        append(sb, INDENT, "    limit(offset + actingBlockLength);");
        append(sb, INDENT, "    builder.append(\"[" + name + "](sbeTemplateId=\");");
        append(sb, INDENT, "    builder.append(TEMPLATE_ID);");
        append(sb, INDENT, "    builder.append(\"|sbeSchemaId=\");");
        append(sb, INDENT, "    builder.append(SCHEMA_ID);");
        append(sb, INDENT, "    builder.append(\"|sbeSchemaVersion=\");");
        append(sb, INDENT, "    if (parentMessage.actingVersion != SCHEMA_VERSION)");
        append(sb, INDENT, "    {");
        append(sb, INDENT, "        builder.append(parentMessage.actingVersion);");
        append(sb, INDENT, "        builder.append('/');");
        append(sb, INDENT, "    }");
        append(sb, INDENT, "    builder.append(SCHEMA_VERSION);");
        append(sb, INDENT, "    builder.append(\"|sbeBlockLength=\");");
        append(sb, INDENT, "    if (actingBlockLength != BLOCK_LENGTH)");
        append(sb, INDENT, "    {");
        append(sb, INDENT, "        builder.append(actingBlockLength);");
        append(sb, INDENT, "        builder.append('/');");
        append(sb, INDENT, "    }");
        append(sb, INDENT, "    builder.append(BLOCK_LENGTH);");
        append(sb, INDENT, "    builder.append(\"):\");");
        appendDecoderDisplay(sb, tokens, groups, varData, INDENT + INDENT);
        sb.append('\n');
        append(sb, INDENT, "    limit(originalLimit);");
        sb.append('\n');
        append(sb, INDENT, "    return builder;");
        append(sb, INDENT, "}");
    }

    private void appendGroupInstanceDecoderDisplay(
        final StringBuilder sb,
        final List<Token> fields,
        final List<Token> groups,
        final List<Token> varData,
        final String baseIndent)
    {
        final String indent = baseIndent + INDENT;

        sb.append('\n');
        append(sb, indent, "public StringBuilder appendTo(final StringBuilder builder)");
        append(sb, indent, "{");
        append(sb, indent, "    if (null == buffer)");
        append(sb, indent, "    {");
        append(sb, indent, "        return builder;");
        append(sb, indent, "    }");
        sb.append('\n');
        Separator.BEGIN_COMPOSITE.appendToGeneratedBuilder(sb, indent + INDENT);
        appendDecoderDisplay(sb, fields, groups, varData, indent + INDENT);
        Separator.END_COMPOSITE.appendToGeneratedBuilder(sb, indent + INDENT);
        sb.append('\n');
        append(sb, indent, "    return builder;");
        append(sb, indent, "}");
    }

    private void appendDecoderDisplay(
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
                ++i;
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
            final String groupDecoderName = decoderName(groupToken.name());

            append(
                sb, indent, "builder.append(\"" + groupName + Separator.KEY_VALUE + Separator.BEGIN_GROUP + "\");");
            append(sb, indent, "final int " + groupName + "OriginalOffset = " + groupName + ".offset;");
            append(sb, indent, "final int " + groupName + "OriginalIndex = " + groupName + ".index;");
            append(sb, indent, "final " + groupDecoderName + " " + groupName + " = this." + groupName + "();");

            append(sb, indent, "if (" + groupName + ".count() > 0)");
            append(sb, indent, "{");
            append(sb, indent, "    while (" + groupName + ".hasNext())");
            append(sb, indent, "    {");
            append(sb, indent, "        " + groupName + ".next().appendTo(builder);");
            Separator.ENTRY.appendToGeneratedBuilder(sb, indent + INDENT + INDENT);
            append(sb, indent, "    }");
            append(sb, indent, "    builder.setLength(builder.length() - 1);");
            append(sb, indent, "}");

            append(sb, indent, groupName + ".offset = " + groupName + "OriginalOffset;");
            append(sb, indent, groupName + ".index = " + groupName + "OriginalIndex;");
            Separator.END_GROUP.appendToGeneratedBuilder(sb, indent);


            lengthBeforeLastGeneratedSeparator = sb.length();
            Separator.FIELD.appendToGeneratedBuilder(sb, indent);

            i = findEndSignal(groups, i, Signal.END_GROUP, groupToken.name());
        }

        for (int i = 0, size = varData.size(); i < size;)
        {
            final Token varDataToken = varData.get(i);
            if (varDataToken.signal() != Signal.BEGIN_VAR_DATA)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_VAR_DATA: token=" + varDataToken);
            }

            final String characterEncoding = varData.get(i + 3).encoding().characterEncoding();
            final String varDataName = formatPropertyName(varDataToken.name());
            append(sb, indent, "builder.append(\"" + varDataName + Separator.KEY_VALUE + "\");");
            if (null == characterEncoding)
            {
                final String name = Generators.toUpperFirstChar(varDataToken.name());
                append(sb, indent, "builder.append(skip" + name + "()).append(\" bytes of raw data\");");
            }
            else
            {
                if (isAsciiEncoding(characterEncoding))
                {
                    append(sb, indent, "builder.append('\\'');");
                    append(sb, indent, formatGetterName(varDataToken.name()) + "(builder);");
                    append(sb, indent, "builder.append('\\'');");
                }
                else
                {
                    append(sb, indent, "builder.append('\\'').append(" + varDataName + "()).append('\\'');");
                }
            }

            lengthBeforeLastGeneratedSeparator = sb.length();
            Separator.FIELD.appendToGeneratedBuilder(sb, indent);

            i += varDataToken.componentTokenCount();
        }

        if (-1 != lengthBeforeLastGeneratedSeparator)
        {
            sb.setLength(lengthBeforeLastGeneratedSeparator);
        }
    }

    private int writeTokenDisplay(
        final String fieldName, final Token typeToken, final StringBuilder sb, final String indent)
    {
        if (typeToken.encodedLength() <= 0 || typeToken.isConstantEncoding())
        {
            return -1;
        }

        append(sb, indent, "builder.append(\"" + fieldName + Separator.KEY_VALUE + "\");");

        switch (typeToken.signal())
        {
            case ENCODING:
                if (typeToken.arrayLength() > 1)
                {
                    if (typeToken.encoding().primitiveType() == PrimitiveType.CHAR)
                    {
                        append(sb, indent,
                            "for (int i = 0; i < " + fieldName + "Length() && this." + fieldName + "(i) > 0; i++)");
                        append(sb, indent, "{");
                        append(sb, indent, "    builder.append((char)this." + fieldName + "(i));");
                        append(sb, indent, "}");
                    }
                    else
                    {
                        Separator.BEGIN_ARRAY.appendToGeneratedBuilder(sb, indent);
                        append(sb, indent, "if (" + fieldName + "Length() > 0)");
                        append(sb, indent, "{");
                        append(sb, indent, "    for (int i = 0; i < " + fieldName + "Length(); i++)");
                        append(sb, indent, "    {");
                        append(sb, indent, "        builder.append(this." + fieldName + "(i));");
                        Separator.ENTRY.appendToGeneratedBuilder(sb, indent + INDENT + INDENT);
                        append(sb, indent, "    }");
                        append(sb, indent, "    builder.setLength(builder.length() - 1);");
                        append(sb, indent, "}");
                        Separator.END_ARRAY.appendToGeneratedBuilder(sb, indent);
                    }
                }
                else
                {
                    // have to duplicate because of checkstyle :/
                    append(sb, indent, "builder.append(this." + fieldName + "());");
                }
                break;

            case BEGIN_ENUM:
                append(sb, indent, "builder.append(this." + fieldName + "());");
                break;

            case BEGIN_SET:
            case BEGIN_COMPOSITE:
            {
                final String typeName = formatClassName(decoderName(typeToken.applicableTypeName()));
                append(sb, indent, "final " + typeName + " " + fieldName + " = this." + fieldName + "();");
                append(sb, indent, "if (null != " + fieldName + ")");
                append(sb, indent, "{");
                append(sb, indent, "    " + fieldName + ".appendTo(builder);");
                append(sb, indent, "}");
                append(sb, indent, "else");
                append(sb, indent, "{");
                append(sb, indent, "    builder.append(\"null\");");
                append(sb, indent, "}");
                break;
            }

            default:
                break;
        }

        final int lengthBeforeFieldSeparator = sb.length();
        Separator.FIELD.appendToGeneratedBuilder(sb, indent);

        return lengthBeforeFieldSeparator;
    }

    private void appendToString(final StringBuilder sb)
    {
        sb.append('\n');
        append(sb, INDENT, "public String toString()");
        append(sb, INDENT, "{");
        append(sb, INDENT, "    if (null == buffer)");
        append(sb, INDENT, "    {");
        append(sb, INDENT, "        return \"\";");
        append(sb, INDENT, "    }");
        sb.append('\n');
        append(sb, INDENT, "    return appendTo(new StringBuilder()).toString();");
        append(sb, INDENT, "}");
    }

    private void appendMessageToString(final StringBuilder sb, final String decoderName)
    {
        sb.append('\n');
        append(sb, INDENT, "public String toString()");
        append(sb, INDENT, "{");
        append(sb, INDENT, "    if (null == buffer)");
        append(sb, INDENT, "    {");
        append(sb, INDENT, "        return \"\";");
        append(sb, INDENT, "    }");
        sb.append('\n');
        append(sb, INDENT, "    final " + decoderName + " decoder = new " + decoderName + "();");
        append(sb, INDENT, "    decoder.wrap(buffer, offset, actingBlockLength, actingVersion);");
        sb.append('\n');
        append(sb, INDENT, "    return decoder.appendTo(new StringBuilder()).toString();");
        append(sb, INDENT, "}");
    }

    private void generateMessageLength(
        final StringBuilder sb,
        final String className,
        final boolean isParent,
        final List<Token> groups,
        final List<Token> varData,
        final String baseIndent)
    {
        final String methodIndent = baseIndent + INDENT;
        final String bodyIndent = methodIndent + INDENT;

        append(sb, methodIndent, "");
        append(sb, methodIndent, "public " + className + " sbeSkip()");
        append(sb, methodIndent, "{");

        if (isParent)
        {
            append(sb, bodyIndent, "sbeRewind();");
        }

        for (int i = 0, size = groups.size(); i < size; i++)
        {
            final Token groupToken = groups.get(i);
            if (groupToken.signal() != Signal.BEGIN_GROUP)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_GROUP: token=" + groupToken);
            }

            final String groupName = formatPropertyName(groupToken.name());
            final String groupDecoderName = decoderName(groupToken.name());

            append(sb, bodyIndent, groupDecoderName + " " + groupName + " = this." + groupName + "();");
            append(sb, bodyIndent, "if (" + groupName + ".count() > 0)");
            append(sb, bodyIndent, "{");
            append(sb, bodyIndent, "    while (" + groupName + ".hasNext())");
            append(sb, bodyIndent, "    {");
            append(sb, bodyIndent, "        " + groupName + ".next();");
            append(sb, bodyIndent, "        " + groupName + ".sbeSkip();");
            append(sb, bodyIndent, "    }");
            append(sb, bodyIndent, "}");
            i = findEndSignal(groups, i, Signal.END_GROUP, groupToken.name());
        }

        for (int i = 0, size = varData.size(); i < size;)
        {
            final Token varDataToken = varData.get(i);
            if (varDataToken.signal() != Signal.BEGIN_VAR_DATA)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_VAR_DATA: token=" + varDataToken);
            }

            final String varDataName = formatPropertyName(varDataToken.name());
            append(sb, bodyIndent, "skip" + Generators.toUpperFirstChar(varDataName) + "();");

            i += varDataToken.componentTokenCount();
        }

        sb.append('\n');
        append(sb, bodyIndent, "return this;");
        append(sb, methodIndent, "}");
    }

    private static String validateBufferImplementation(
        final String fullyQualifiedBufferImplementation, final Class<?> bufferClass)
    {
        Verify.notNull(fullyQualifiedBufferImplementation, "fullyQualifiedBufferImplementation");

        try
        {
            final Class<?> clazz = Class.forName(fullyQualifiedBufferImplementation);
            if (!bufferClass.isAssignableFrom(clazz))
            {
                throw new IllegalArgumentException(
                    fullyQualifiedBufferImplementation + " doesn't implement " + bufferClass.getName());
            }

            return clazz.getSimpleName();
        }
        catch (final ClassNotFoundException ex)
        {
            throw new IllegalArgumentException("Unable to find " + fullyQualifiedBufferImplementation, ex);
        }
    }

    private String encoderName(final String className)
    {
        return formatClassName(className) + "Encoder";
    }

    private String decoderName(final String className)
    {
        return formatClassName(className) + "Decoder";
    }

    private String implementsInterface(final String interfaceName)
    {
        return shouldGenerateInterfaces ? " implements " + interfaceName : "";
    }
}
