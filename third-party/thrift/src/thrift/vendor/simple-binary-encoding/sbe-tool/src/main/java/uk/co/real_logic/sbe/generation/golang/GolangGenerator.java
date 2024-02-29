/*
 * Copyright 2013-2024 Real Logic Limited.
 * Copyright (C) 2016 MarketFactory, Inc
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
package uk.co.real_logic.sbe.generation.golang;

import uk.co.real_logic.sbe.PrimitiveType;
import uk.co.real_logic.sbe.generation.CodeGenerator;
import org.agrona.generation.OutputManager;
import uk.co.real_logic.sbe.generation.Generators;
import uk.co.real_logic.sbe.generation.java.JavaUtil;
import uk.co.real_logic.sbe.ir.*;
import org.agrona.Verify;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.Writer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;
import java.util.Stack;
import java.util.TreeSet;

import static uk.co.real_logic.sbe.PrimitiveType.CHAR;
import static uk.co.real_logic.sbe.generation.Generators.toUpperFirstChar;
import static uk.co.real_logic.sbe.generation.golang.GolangUtil.*;
import static uk.co.real_logic.sbe.ir.GenerationUtil.collectVarData;
import static uk.co.real_logic.sbe.ir.GenerationUtil.collectGroups;
import static uk.co.real_logic.sbe.ir.GenerationUtil.collectFields;

/**
 * Codec generator for the Go Lang programming language.
 */
@SuppressWarnings("MethodLength")
public class GolangGenerator implements CodeGenerator
{
    private final Ir ir;
    private final OutputManager outputManager;

    private final Stack<TreeSet<String>> imports = new Stack<>();

    /**
     * Create a new Go language {@link CodeGenerator}.
     *
     * @param ir            for the messages and types.
     * @param outputManager for generating the codecs to.
     */
    public GolangGenerator(final Ir ir, final OutputManager outputManager)
    {
        Verify.notNull(ir, "ir");
        Verify.notNull(outputManager, "outputManager");

        this.ir = ir;
        this.outputManager = outputManager;
        this.imports.push(new TreeSet<>());  // ensure at least one
    }

    /**
     * Generate a file for the Ir based on a template.
     *
     * @param fileName     to generate.
     * @param templateName for the file.
     * @throws IOException if an error is encountered when writing the output.
     */
    public void generateFileFromTemplate(final String fileName, final String templateName) throws IOException
    {
        try (Writer out = outputManager.createOutput(fileName))
        {
            out.append(generateFromTemplate(ir.namespaces(), templateName));
        }
    }

    /**
     * Generate the stubs for the types used as message fields.
     *
     * @throws IOException if an error is encountered when writing the output.
     */
    public void generateTypeStubs() throws IOException
    {
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
                    generateComposite(tokens, "");
                    break;

                default:
                    break;
            }
        }
    }

    // MessageHeader is special but the standard allows it to be
    // pretty arbitrary after the first four fields.
    // All we need is the imports, type declaration, and encode/decode.
    /**
     * Generate the composites for dealing with the message header.
     *
     * @throws IOException if an error is encountered when writing the output.
     */
    public void generateMessageHeaderStub() throws IOException
    {
        final String messageHeader = "MessageHeader";
        try (Writer out = outputManager.createOutput(messageHeader))
        {
            final StringBuilder sb = new StringBuilder();
            final List<Token> tokens = ir.headerStructure().tokens();

            imports.push(new TreeSet<>());
            imports.peek().add("io");

            generateTypeDeclaration(sb, messageHeader);
            generateTypeBodyComposite(sb, messageHeader, tokens.subList(1, tokens.size() - 1));

            generateEncodeDecode(sb, messageHeader, tokens.subList(1, tokens.size() - 1), false, false);
            generateCompositePropertyElements(sb, messageHeader, tokens.subList(1, tokens.size() - 1));
            out.append(generateFileHeader(ir.namespaces()));
            out.append(sb);
            imports.pop();
        }
    }

    /**
     * {@inheritDoc}
     */
    public void generate() throws IOException
    {
        // Add the Marshalling from the big or little endian
        // template kept inside the jar.
        //
        // The MessageHeader structure along with its en/decoding is
        // also in the templated SbeMarshalling.go
        //
        // final Token token = ir.messages().iterator().next().get(0);
        if (ir.byteOrder() == ByteOrder.LITTLE_ENDIAN)
        {
            generateFileFromTemplate("SbeMarshalling", "SbeMarshallingLittleEndian");
        }
        else
        {
            generateFileFromTemplate("SbeMarshalling", "SbeMarshallingBigEndian");
        }

        generateMessageHeaderStub();
        generateTypeStubs();

        for (final List<Token> tokens : ir.messages())
        {
            final Token msgToken = tokens.get(0);
            final String typeName = formatTypeName(msgToken.name());

            try (Writer out = outputManager.createOutput(typeName))
            {
                final StringBuilder sb = new StringBuilder();

                imports.push(new TreeSet<>());
                imports.peek().add("io");

                generateTypeDeclaration(sb, typeName);
                generateTypeBody(sb, typeName, tokens.subList(1, tokens.size() - 1));

                generateMessageCode(sb, typeName, tokens);

                final List<Token> messageBody = tokens.subList(1, tokens.size() - 1);
                int i = 0;

                final List<Token> fields = new ArrayList<>();
                i = collectFields(messageBody, i, fields);

                final List<Token> groups = new ArrayList<>();
                i = collectGroups(messageBody, i, groups);

                final List<Token> varData = new ArrayList<>();
                collectVarData(messageBody, i, varData);

                generateFields(sb, typeName, fields);
                generateGroups(sb, groups, typeName);
                generateGroupProperties(sb, groups, typeName);
                generateVarData(sb, typeName, varData);

                out.append(generateFileHeader(ir.namespaces()));
                out.append(sb);
                imports.pop();
            }
        }
    }

    private String generateEncodeOffset(final int gap, final String indent)
    {
        if (gap > 0)
        {
            return String.format("\n" +
                "%1$s\tfor i := 0; i < %2$d; i++ {\n" +
                "%1$s\t\tif err := _m.WriteUint8(_w, uint8(0)); err != nil {\n" +
                "%1$s\t\t\treturn err\n" +
                "%1$s\t\t}\n" +
                "%1$s\t}\n",
                indent,
                gap);
        }
        return "";
    }

    private String generateDecodeOffset(final int gap, final String indent)
    {
        if (gap > 0)
        {
            imports.peek().add("io");
            imports.peek().add("io/ioutil");
            return String.format("%1$s\tio.CopyN(ioutil.Discard, _r, %2$d)\n", indent, gap);
        }

        return "";
    }

    private void generateCharacterEncodingRangeCheck(
        final StringBuilder sb,
        final String varName,
        final Token token)
    {
        final String characterEncoding = token.encoding().characterEncoding();

        if (null != characterEncoding)
        {
            if (JavaUtil.isAsciiEncoding(characterEncoding))
            {
                imports.peek().add("fmt");
                sb.append(String.format(
                    "\tfor idx, ch := range %1$s {\n" +
                    "\t\tif ch > 127 {\n" +
                    "\t\t\treturn fmt.Errorf(\"%1$s[%%d]=%%d" +
                    " failed ASCII validation\", idx, ch)\n" +
                    "\t\t}\n" +
                    "\t}\n",
                    varName));
            }
            else if (JavaUtil.isUtf8Encoding(characterEncoding))
            {
                imports.peek().add("errors");
                imports.peek().add("unicode/utf8");
                sb.append(String.format(
                    "\tif !utf8.Valid(%1$s[:]) {\n" +
                    "\t\treturn errors.New(\"%1$s failed UTF-8 validation\")\n" +
                    "\t}\n",
                    varName));
            }
            else
            {
                throw new IllegalArgumentException("Unsupported encoding: " + characterEncoding);
            }
        }
    }

    private void generateEncodePrimitive(
        final StringBuilder sb,
        final char varName,
        final String propertyName,
        final Token encodingToken)
    {
        final PrimitiveType primitiveType = encodingToken.encoding().primitiveType();
        final String marshalType = golangMarshalType(primitiveType);

        // Complexity lurks here
        // A single character (byte) is handled as a uint8 (equivalent to byte)
        // An array of uint8 or byte is handled as Bytes (for speed)

        if (primitiveType == PrimitiveType.CHAR || primitiveType == PrimitiveType.UINT8)
        {
            if (encodingToken.arrayLength() > 1)
            {
                // byte or uint8 arrays get treated as Bytes
                // We take a slice to make the type right
                sb.append(String.format(
                    "\tif err := _m.WriteBytes(_w, %1$s.%2$s[:]); err != nil {\n" +
                    "\t\treturn err\n" +
                    "\t}\n",
                    varName,
                    propertyName));
            }
            else
            {
                // A single byte or uint8 gets treated as a uint8
                sb.append(String.format(
                    "\tif err := _m.WriteUint8(_w, %1$s.%2$s); err != nil {\n" +
                    "\t\treturn err\n" +
                    "\t}\n",
                    varName,
                    propertyName));
            }
        }
        else
        {
            if (encodingToken.arrayLength() > 1)
            {
                // Other array types need a for loop
                sb.append(String.format(
                    "\tfor idx := 0; idx < %1$d; idx++ {\n" +
                    "\t\tif err := _m.Write%2$s(_w, %3$s.%4$s[idx]); err != nil {\n" +
                    "\t\t\treturn err\n" +
                    "\t\t}\n" +
                    "\t}\n",
                    encodingToken.arrayLength(),
                    marshalType,
                    varName,
                    propertyName));
            }
            else
            {
                sb.append(String.format(
                    "\tif err := _m.Write%1$s(_w, %2$s.%3$s); err != nil {\n" +
                    "\t\treturn err\n" +
                    "\t}\n",
                    marshalType,
                    varName,
                    propertyName));
            }
        }
    }

    private void generateDecodePrimitive(final StringBuilder sb, final String varName, final Token token)
    {
        final PrimitiveType primitiveType = token.encoding().primitiveType();
        final String marshalType = golangMarshalType(primitiveType);

        // Complexity lurks here
        // A single character (byte) is handled as a uint8 (equivalent to byte)
        // An array of uint8 or byte is handled as Bytes (for speed)
        // And then we need to deal with constant encodings
        // Finally don't forget sinceVersion as we might get default values

        // Decode of a constant is simply assignment
        if (token.isConstantEncoding())
        {
            // if primitiveType="char" this is a character array
            if (primitiveType == CHAR)
            {
                if (token.encoding().constValue().size() > 1)
                {
                    // constValue is a string
                    sb.append(String.format(
                        "\tcopy(%1$s[:], \"%2$s\")\n",
                        varName,
                        token.encoding().constValue()));
                }
                else
                {
                    // constValue is a char
                    sb.append(String.format(
                        "\t%1$s[0] = %2$s\n",
                        varName,
                        token.encoding().constValue()));
                }
            }
            else
            {
                sb.append(String.format(
                    "\t%1$s = %2$s\n",
                    varName,
                    generateLiteral(primitiveType, token.encoding().constValue().toString())));
            }
        }
        else
        {
            if (primitiveType == PrimitiveType.CHAR || primitiveType == PrimitiveType.UINT8)
            {
                if (token.arrayLength() > 1)
                {
                    sb.append(String.format(
                        "\tif !%1$sInActingVersion(actingVersion) {\n" +
                        "\t\tfor idx := 0; idx < %2$s; idx++ {\n" +
                        "\t\t\t%1$s[idx] = %1$sNullValue()\n" +
                        "\t\t}\n" +
                        "\t} else {\n" +
                        "\t\tif err := _m.ReadBytes(_r, %1$s[:]); err != nil {\n" +
                        "\t\t\treturn err\n" +
                        "\t\t}\n" +
                        "\t}\n",
                        varName,
                        token.arrayLength()));
                }
                else
                {
                    // A single byte or uint8 gets treated as a uint8
                    sb.append(String.format(
                        "\tif !%1$sInActingVersion(actingVersion) {\n" +
                        "\t\t%1$s = %1$sNullValue()\n" +
                        "\t} else {\n" +
                        "\t\tif err := _m.ReadUint8(_r, &%1$s); err != nil {\n" +
                        "\t\t\treturn err\n" +
                        "\t\t}\n" +
                        "\t}\n",
                        varName));
                }
            }
            else
            {
                if (token.arrayLength() > 1)
                {
                    // Other array types need a for loop
                    sb.append(String.format(
                        "\tif !%2$sInActingVersion(actingVersion) {\n" +
                        "\t\tfor idx := 0; idx < %1$d; idx++ {\n" +
                        "\t\t\t%2$s[idx] = %2$sNullValue()\n" +
                        "\t\t}\n" +
                        "\t} else {\n" +
                        "\t\tfor idx := 0; idx < %1$d; idx++ {\n" +
                        "\t\t\tif err := _m.Read%3$s(_r, &%2$s[idx]); err != nil {\n" +
                        "\t\t\t\treturn err\n" +
                        "\t\t\t}\n" +
                        "\t\t}\n" +
                        "\t}\n",
                        token.arrayLength(),
                        varName,
                        marshalType));
                }
                else
                {
                    sb.append(String.format(
                        "\tif !%1$sInActingVersion(actingVersion) {\n" +
                        "\t\t%1$s = %1$sNullValue()\n" +
                        "\t} else {\n" +
                        "\t\tif err := _m.Read%2$s(_r, &%1$s); err != nil {\n" +
                        "\t\t\treturn err\n" +
                        "\t\t}\n" +
                        "\t}\n",
                        varName,
                        marshalType));
                }
            }
        }
    }

    private void generateRangeCheckPrimitive(
        final StringBuilder sb,
        final String varName,
        final Token token,
        final Boolean isOptional)
    {
        // Note that constant encoding is a property of the type
        // and so works on signal/encoding tokens but optionality is
        // a property of the field and so is not set on the encoding token.
        // For this reason we can use the token to check for constancy
        // but pass in optionality as a parameter

        // Constant values don't need checking
        if (token.isConstantEncoding())
        {
            return;
        }

        // If this field is unknown then we have nothing to check
        // Otherwise do the Min,MaxValue checks (possibly for arrays)
        imports.peek().add("fmt");
        if (token.arrayLength() > 1)
        {
            sb.append(String.format(
                "\tif %1$sInActingVersion(actingVersion) {\n" +
                "\t\tfor idx := 0; idx < %2$s; idx++ {\n" +
                "\t\t\tif %1$s[idx] < %1$sMinValue() || %1$s[idx] > %1$sMaxValue() {\n" +
                "\t\t\t\treturn fmt.Errorf(\"Range check failed on %1$s[%%d] " +
                "(%%v < %%v > %%v)\", idx, %1$sMinValue(), %1$s[idx], %1$sMaxValue())\n" +
                "\t\t\t}\n" +
                "\t\t}\n" +
                "\t}\n",
                varName,
                token.arrayLength()));
        }
        else
        {
            // Optional fields can be NullValue which may be outside the
            // range of Min->Max so we need a special case.
            // Structured this way for go fmt sanity on both cases.
            final String check;
            if (isOptional)
            {
                check = "\t\tif %1$s != %1$sNullValue() && (%1$s < %1$sMinValue() || %1$s > %1$sMaxValue()) {\n";
            }
            else
            {
                check = "\t\tif %1$s < %1$sMinValue() || %1$s > %1$sMaxValue() {\n";
            }

            sb.append(String.format(
                "\tif %1$sInActingVersion(actingVersion) {\n" +
                check +
                "\t\t\treturn fmt.Errorf(\"Range check failed on %1$s " +
                "(%%v < %%v > %%v)\", %1$sMinValue(), %1$s, %1$sMaxValue())\n" +
                "\t\t}\n" +
                "\t}\n",
                varName));
        }

        // Fields that are an [n]byte may have a characterEncoding which
        // should also be checked
        if (token.arrayLength() > 1 && token.encoding().primitiveType() == PrimitiveType.CHAR)
        {
            generateCharacterEncodingRangeCheck(sb, varName, token);
        }
    }

    private void generateOptionalInitPrimitive(
        final StringBuilder sb,
        final String varName,
        final Token token)
    {
        final Encoding encoding = token.encoding();

        // Optional items get initialized to their NullValue
        if (token.arrayLength() > 1)
        {
            sb.append(String.format(
                "\tfor idx := 0; idx < %1$d; idx++ {\n" +
                "\t\t%2$s[idx] = %3$s\n" +
                "\t}\n",
                token.arrayLength(),
                varName,
                generateNullValueLiteral(encoding.primitiveType(), encoding)));
        }
        else
        {
            sb.append(String.format(
                "\t%1$s = %2$s\n",
                varName,
                generateNullValueLiteral(encoding.primitiveType(), encoding)));
        }
    }

    private void generateConstantInitPrimitive(
        final StringBuilder sb,
        final String varName,
        final Token token)
    {
        final Encoding encoding = token.encoding();

        // Decode of a constant is simply assignment
        if (token.isConstantEncoding())
        {
            // if primitiveType="char" this is a character array
            if (encoding.primitiveType() == CHAR)
            {
                if (encoding.constValue().size() > 1)
                {
                    // constValue is a string
                    sb.append(String.format(
                        "\tcopy(%1$s[:], \"%2$s\")\n",
                        varName,
                        encoding.constValue()));
                }
                else
                {
                    // constValue is a char
                    sb.append(String.format(
                        "\t%1$s[0] = %2$s\n",
                        varName,
                        encoding.constValue()));
                }
            }
            else
            {
                sb.append(String.format(
                    "\t%1$s = %2$s\n",
                    varName,
                    generateLiteral(encoding.primitiveType(), encoding.constValue().toString())));
            }
        }
    }

    private void generateEncodeDecodeOpen(
        final StringBuilder encode,
        final StringBuilder decode,
        final StringBuilder rangeCheck,
        final StringBuilder init,
        final char varName,
        final String typeName,
        final Boolean isMessage,
        final Boolean isExtensible)
    {
        generateEncodeHeader(encode, varName, typeName, isMessage, false);
        generateDecodeHeader(decode, varName, typeName, isMessage, isExtensible);
        generateRangeCheckHeader(rangeCheck, varName, typeName, false);
        generateInitHeader(init, varName, typeName);
    }

    private void generateEncodeDecodeClose(
        final StringBuilder encode,
        final StringBuilder decode,
        final StringBuilder rangeCheck,
        final StringBuilder init)
    {
        encode.append("\treturn nil\n}\n");
        decode.append("\treturn nil\n}\n");
        rangeCheck.append("\treturn nil\n}\n");
        init.append("\treturn\n}\n");
    }

    // Newer messages and groups can add extra properties before the variable
    // length elements (groups and varData). We read past the difference
    // between the message's blockLength and our (older) schema's blockLength
    private void generateExtensionCheck(
        final StringBuilder sb,
        final char varName)
    {
        imports.peek().add("io");
        imports.peek().add("io/ioutil");
        sb.append(String.format(
            "\tif actingVersion > %1$s.SbeSchemaVersion() && blockLength > %1$s.SbeBlockLength() {\n" +
            "\t\tio.CopyN(ioutil.Discard, _r, int64(blockLength-%1$s.SbeBlockLength()))\n" +
            "\t}\n",
            varName));
    }

    // Returns the size of the last Message/Group
    private int generateEncodeDecode(
        final StringBuilder sb,
        final String typeName,
        final List<Token> tokens,
        final boolean isMessage,
        final boolean isExtensible)
    {
        final char varName = Character.toLowerCase(typeName.charAt(0));
        final StringBuilder encode = new StringBuilder();
        final StringBuilder decode = new StringBuilder();
        final StringBuilder init = new StringBuilder();
        final StringBuilder rangeCheck = new StringBuilder();
        final StringBuilder nested = new StringBuilder();
        int currentOffset = 0;
        int gap;
        boolean extensionStarted = false;

        // Open all our methods
        generateEncodeDecodeOpen(encode, decode, rangeCheck, init, varName, typeName, isMessage, isExtensible);

        for (int i = 0; i < tokens.size(); i++)
        {
            final Token signalToken = tokens.get(i);
            final String propertyName = formatPropertyName(signalToken.name());

            switch (signalToken.signal())
            {
                case BEGIN_MESSAGE: // Check range *before* we encode setting the acting version to schema version
                    encode.append(String.format(
                        "\tif doRangeCheck {\n" +
                        "\t\tif err := %1$s.RangeCheck(%1$s.SbeSchemaVersion(), %1$s.SbeSchemaVersion());" +
                        " err != nil {\n" +
                        "\t\t\treturn err\n" +
                        "\t\t}\n" +
                        "\t}\n",
                        varName));
                    break;

                case END_MESSAGE:
                    // Newer version extra fields check
                    if (isExtensible && !extensionStarted)
                    {
                        generateExtensionCheck(decode, varName);
                        extensionStarted = true;
                    }

                    // Check range *after* we decode using the acting
                    // version of the encoded message
                    decode.append(String.format(
                        "\tif doRangeCheck {\n" +
                        "\t\tif err := %1$s.RangeCheck(actingVersion, %1$s.SbeSchemaVersion()); err != nil {\n" +
                        "\t\t\treturn err\n" +
                        "\t\t}\n" +
                        "\t}\n",
                        varName));
                    break;

                case BEGIN_ENUM:
                case BEGIN_SET:
                    currentOffset += generatePropertyEncodeDecode(
                        signalToken, typeName, encode, decode, currentOffset);
                    i += signalToken.componentTokenCount() - 1;
                    break;

                case BEGIN_COMPOSITE:
                    currentOffset += generatePropertyEncodeDecode(
                        signalToken, typeName, encode, decode, currentOffset);
                    i += signalToken.componentTokenCount() - 1;

                    rangeCheck.append(String.format(
                        "\tif err := %1$s.%2$s.RangeCheck(actingVersion, schemaVersion); err != nil {\n" +
                        "\t\treturn err\n" +
                        "\t}\n",
                        varName, propertyName));
                    break;

                case BEGIN_FIELD:
                    if (tokens.size() >= i + 1)
                    {
                        currentOffset += generateFieldEncodeDecode(
                            tokens.subList(i, tokens.size() - 1),
                            varName, currentOffset, encode, decode, rangeCheck, init);

                        // Encodings just move past the encoding token
                        if (tokens.get(i + 1).signal() == Signal.ENCODING)
                        {
                            i += 1;
                        }
                        else
                        {
                            i += signalToken.componentTokenCount() - 1;
                        }
                    }
                    break;

                case ENCODING:
                    gap = signalToken.offset() - currentOffset;
                    encode.append(generateEncodeOffset(gap, ""));
                    decode.append(generateDecodeOffset(gap, ""));
                    currentOffset += signalToken.encodedLength() + gap;
                    final String primitive = varName + "." + propertyName;

                    // Encode of a constant is a nullop and we want to
                    // initialize constant values.
                    if (signalToken.isConstantEncoding())
                    {
                        generateConstantInitPrimitive(init, primitive, signalToken);
                    }
                    else
                    {
                        generateEncodePrimitive(encode, varName, formatPropertyName(signalToken.name()), signalToken);
                    }

                    // Optional tokens also get initialized
                    if (signalToken.isOptionalEncoding())
                    {
                        generateOptionalInitPrimitive(init, primitive, signalToken);
                    }

                    generateDecodePrimitive(decode, primitive, signalToken);
                    generateRangeCheckPrimitive(rangeCheck, primitive, signalToken, signalToken.isOptionalEncoding());
                    break;

                case BEGIN_GROUP:
                    // Newer version extra fields check
                    if (isExtensible && !extensionStarted)
                    {
                        generateExtensionCheck(decode, varName);
                        extensionStarted = true;
                    }

                    // Write the group, saving any extra offset we need to skip
                    currentOffset += generateGroupEncodeDecode(
                        tokens.subList(i, tokens.size() - 1),
                        typeName,
                        encode, decode, rangeCheck, currentOffset);

                    // Recurse
                    gap = Math.max(0,
                        signalToken.encodedLength() -
                            generateEncodeDecode(
                                nested,
                                typeName + toUpperFirstChar(signalToken.name()),
                                tokens.subList(i + 5, tokens.size() - 1),
                                false, true));

                    // Group gap block length handling
                    encode.append(generateEncodeOffset(gap, "\t")).append("\t}\n");
                    decode.append(generateDecodeOffset(gap, "\t")).append("\t}\n");

                    // And we can move over this group to the END_GROUP
                    i += signalToken.componentTokenCount() - 1;
                    break;

                case END_GROUP:
                    // Newer version extra fields check
                    if (isExtensible && !extensionStarted)
                    {
                        generateExtensionCheck(decode, varName);
                    }
                    // Close out this group and unwind
                    generateEncodeDecodeClose(encode, decode, rangeCheck, init);
                    sb.append(encode).append(decode).append(rangeCheck).append(init).append(nested);
                    return currentOffset; // for gap calculations

                case BEGIN_VAR_DATA:
                    // Newer version extra fields check
                    if (isExtensible && !extensionStarted)
                    {
                        generateExtensionCheck(decode, varName);
                        extensionStarted = true;
                    }
                    currentOffset += generateVarDataEncodeDecode(
                        tokens.subList(i, tokens.size() - 1),
                        typeName,
                        encode, decode, rangeCheck, currentOffset);
                    // And we can move over this group
                    i += signalToken.componentTokenCount() - 1;
                    break;

                default:
                    break;
            }
        }

        // You can use blockLength on both messages and groups (handled above)
        // to leave some space (akin to an offset).
        final Token endToken = tokens.get(tokens.size() - 1);
        if (endToken.signal() == Signal.END_MESSAGE)
        {
            gap = endToken.encodedLength() - currentOffset;
            encode.append(generateEncodeOffset(gap, ""));
            decode.append(generateDecodeOffset(gap, ""));
        }

        // Close out the methods and append
        generateEncodeDecodeClose(encode, decode, rangeCheck, init);
        sb.append(encode).append(decode).append(rangeCheck).append(init).append(nested);

        return currentOffset;
    }

    private void generateEnumEncodeDecode(final StringBuilder sb, final String enumName, final Token token)
    {
        final char varName = Character.toLowerCase(enumName.charAt(0));
        final String typeName = golangTypeName(token.encoding().primitiveType());
        final String marshalType;

        // The enum type might be char (golang byte) which we encode
        // as a uint8
        if (token.encoding().primitiveType() == PrimitiveType.CHAR)
        {
            marshalType = "Uint8";
        }
        else
        {
            marshalType = golangMarshalType(token.encoding().primitiveType());
        }

        // Encode
        generateEncodeHeader(sb, varName, enumName + "Enum", false, true);
        sb.append(String.format(
            "\tif err := _m.Write%1$s(_w, %2$s(%3$s)); err != nil {\n" +
            "\t\treturn err\n" +
            "\t}\n" +
            "\treturn nil\n}\n",
            marshalType,
            typeName,
            varName));

        // Decode
        generateDecodeHeader(sb, varName, enumName + "Enum", false, false);
        sb.append(String.format(
            "\tif err := _m.Read%1$s(_r, (*%2$s)(%3$s)); err != nil {\n" +
            "\t\treturn err\n" +
            "\t}\n" +
            "\treturn nil\n}\n",
            marshalType,
            typeName,
            varName));

        // Range check
        // We use golang's reflect to range over the values in the
        // struct to check which are legitimate
        imports.peek().add("fmt");
        imports.peek().add("reflect");
        generateRangeCheckHeader(sb, varName, enumName + "Enum", true);

        // For enums we can add new fields so if we're decoding a
        // newer version then the content is definitionally ok.
        // When encoding actingVersion === schemaVersion
        sb.append(
            "\tif actingVersion > schemaVersion {\n" +
            "\t\treturn nil\n" +
            "\t}\n");

        // Otherwise the value should be known
        sb.append(String.format(
            "\tvalue := reflect.ValueOf(%2$s)\n" +
            "\tfor idx := 0; idx < value.NumField(); idx++ {\n" +
            "\t\tif %1$s == value.Field(idx).Interface() {\n" +
            "\t\t\treturn nil\n" +
            "\t\t}\n" +
            "\t}\n" +
            "\treturn fmt.Errorf(\"Range check failed on %2$s, unknown enumeration value %%d\", %1$s)\n" +
            "}\n",
            varName,
            enumName));
    }

    private void generateChoiceEncodeDecode(final StringBuilder sb, final String choiceName, final Token token)
    {
        final char varName = Character.toLowerCase(choiceName.charAt(0));

        generateEncodeHeader(sb, varName, choiceName, false, false);

        sb.append(String.format(
            "\tvar wireval uint%1$d = 0\n" +
            "\tfor k, v := range %2$s {\n" +
            "\t\tif v {\n" +
            "\t\t\twireval |= (1 << uint(k))\n" +
            "\t\t}\n\t}\n" +
            "\treturn _m.WriteUint%1$d(_w, wireval)\n" +
            "}\n",
            token.encodedLength() * 8,
            varName));

        generateDecodeHeader(sb, varName, choiceName, false, false);

        sb.append(String.format(
            "\tvar wireval uint%1$d\n\n" +
            "\tif err := _m.ReadUint%1$d(_r, &wireval); err != nil {\n" +
            "\t\treturn err\n" +
            "\t}\n" +
            "\n" +
            "\tvar idx uint\n" +
            "\tfor idx = 0; idx < %1$d; idx++ {\n" +
            "\t\t%2$s[idx] = (wireval & (1 << idx)) > 0\n" +
            "\t}\n",
            token.encodedLength() * 8,
            varName));

        sb.append("\treturn nil\n}\n");
    }

    private void generateEncodeHeader(
        final StringBuilder sb,
        final char varName,
        final String typeName,
        final Boolean isMessage,
        final Boolean isEnum)
    {
        // Only messages get the rangeCheck flag
        String messageArgs = "";
        if (isMessage)
        {
            messageArgs = ", doRangeCheck bool";
        }

        sb.append(String.format(
            "\nfunc (%1$s %3$s%2$s) Encode(_m *SbeGoMarshaller, _w io.Writer" +
            messageArgs +
            ") error {\n",
            varName,
            typeName,
            (isEnum ? "" : "*")));
    }

    private void generateDecodeHeader(
        final StringBuilder sb,
        final char varName,
        final String typeName,
        final Boolean isMessage,
        final Boolean isExtensible)
    {
        String decodeArgs = "";
        final String blockLengthType = golangTypeName(ir.headerStructure().blockLengthType());

        // Messages, groups, and varData are extensible so need to know
        // working block length.
        // Messages mandate only 16 bits, otherwise let's be generous and
        // support the platform.
        if (isExtensible)
        {
            if (isMessage)
            {
                decodeArgs += ", blockLength " + blockLengthType;
            }
            else
            {
                decodeArgs += ", blockLength uint";
            }
        }

        // Only messages get the rangeCheck flags
        if (isMessage)
        {
            decodeArgs += ", doRangeCheck bool";
        }

        sb.append(String.format(
            "\nfunc (%1$s *%2$s) Decode(_m *SbeGoMarshaller, _r io.Reader, actingVersion uint16" +
            decodeArgs +
            ") error {\n",
            varName,
            typeName));
    }

    private void generateRangeCheckHeader(
        final StringBuilder sb,
        final char varName,
        final String typeName,
        final boolean isEnum)
    {
        sb.append(String.format(
            "\nfunc (%1$s %3$s%2$s) RangeCheck(actingVersion uint16, schemaVersion uint16) error {\n",
            varName,
            typeName,
            (isEnum ? "" : "*")));
    }

    private void generateInitHeader(
        final StringBuilder sb,
        final char varName,
        final String typeName)
    {
        // Init is a function rather than a method to guarantee uniqueness
        // as a field of a structure may collide
        sb.append(String.format(
            "\nfunc %1$sInit(%2$s *%1$s) {\n",
            typeName,
            varName));
    }

    // Returns how many extra tokens to skip over
    private int generateFieldEncodeDecode(
        final List<Token> tokens,
        final char varName,
        final int currentOffset,
        final StringBuilder encode,
        final StringBuilder decode,
        final StringBuilder rc,
        final StringBuilder init)
    {
        final Token signalToken = tokens.get(0);
        final Token encodingToken = tokens.get(1);
        final String propertyName = formatPropertyName(signalToken.name());

        int gap = 0; // for offset calculations

        switch (encodingToken.signal())
        {
            case BEGIN_COMPOSITE:
            case BEGIN_ENUM:
            case BEGIN_SET:
                gap = signalToken.offset() - currentOffset;
                encode.append(generateEncodeOffset(gap, ""));
                decode.append(generateDecodeOffset(gap, ""));

                // Encode of a constant is a nullop, decode is assignment
                if (signalToken.isConstantEncoding())
                {
                    decode.append(String.format(
                        "\t%1$s.%2$s = %3$s\n",
                        varName, propertyName, signalToken.encoding().constValue()));
                    init.append(String.format(
                        "\t%1$s.%2$s = %3$s\n",
                        varName, propertyName, signalToken.encoding().constValue()));
                }
                else
                {
                    encode.append(String.format(
                        "\tif err := %1$s.%2$s.Encode(_m, _w); err != nil {\n" +
                        "\t\treturn err\n" +
                        "\t}\n",
                        varName, propertyName));

                    decode.append(String.format(
                        "\tif %1$s.%2$sInActingVersion(actingVersion) {\n" +
                        "\t\tif err := %1$s.%2$s.Decode(_m, _r, actingVersion); err != nil {\n" +
                        "\t\t\treturn err\n" +
                        "\t\t}\n" +
                        "\t}\n",
                        varName, propertyName));
                }

                if (encodingToken.signal() == Signal.BEGIN_ENUM)
                {
                    rc.append(String.format(
                        "\tif err := %1$s.%2$s.RangeCheck(actingVersion, schemaVersion); err != nil {\n" +
                        "\t\treturn err\n" +
                        "\t}\n",
                        varName, propertyName));
                }
                break;

            case ENCODING:
                gap = encodingToken.offset() - currentOffset;
                encode.append(generateEncodeOffset(gap, ""));
                decode.append(generateDecodeOffset(gap, ""));
                final String primitive = varName + "." + propertyName;

                // Encode of a constant is a nullop and we want to
                // initialize constant values.
                // (note: constancy is determined by the type's token)
                if (encodingToken.isConstantEncoding())
                {
                    generateConstantInitPrimitive(init, primitive, encodingToken);
                }
                else
                {
                    generateEncodePrimitive(encode, varName, formatPropertyName(signalToken.name()), encodingToken);
                }

                // Optional tokens get initialized to NullValue
                // (note: optionality is determined by the field's token)
                if (signalToken.isOptionalEncoding())
                {
                    generateOptionalInitPrimitive(init, primitive, encodingToken);
                }

                generateDecodePrimitive(decode, primitive, encodingToken);
                generateRangeCheckPrimitive(rc, primitive, encodingToken, signalToken.isOptionalEncoding());
                break;

            default:
                break;
        }

        return encodingToken.encodedLength() + gap;
    }

    // returns how much to add to offset
    private int generatePropertyEncodeDecode(
        final Token token,
        final String typeName,
        final StringBuilder encode,
        final StringBuilder decode,
        final int currentOffset)
    {
        final char varName = Character.toLowerCase(typeName.charAt(0));
        final String propertyName = formatPropertyName(token.name());
        final int gap = token.offset() - currentOffset;
        encode.append(generateEncodeOffset(gap, ""));
        decode.append(generateDecodeOffset(gap, ""));

        encode.append(String.format(
            "\tif err := %1$s.%2$s.Encode(_m, _w); err != nil {\n" +
            "\t\treturn err\n" +
            "\t}\n",
            varName,
            propertyName));

        decode.append(String.format(
            "\tif %1$s.%2$sInActingVersion(actingVersion) {\n" +
            "\t\tif err := %1$s.%2$s.Decode(_m, _r, actingVersion); err != nil {\n" +
            "\t\t\treturn err\n" +
            "\t\t}\n" +
            "\t}\n",
            varName,
            propertyName));

        return token.encodedLength() + gap;
    }

    // returns how much to add to offset
    private int generateVarDataEncodeDecode(
        final List<Token> tokens,
        final String typeName,
        final StringBuilder encode,
        final StringBuilder decode,
        final StringBuilder rc,
        final int currentOffset)
    {
        final Token signalToken = tokens.get(0);
        final char varName = Character.toLowerCase(typeName.charAt(0));
        final String propertyName = formatPropertyName(signalToken.name());

        // Offset handling
        final int gap = Math.max(signalToken.offset() - currentOffset, 0);
        encode.append(generateEncodeOffset(gap, ""));
        decode.append(generateDecodeOffset(gap, ""));

        final String golangTypeForLength = golangTypeName(tokens.get(2).encoding().primitiveType());
        final String golangTypeForLengthMarshal = golangMarshalType(tokens.get(2).encoding().primitiveType());
        final String golangTypeForData = golangTypeName(tokens.get(3).encoding().primitiveType());

        generateCharacterEncodingRangeCheck(rc, varName + "." + propertyName, tokens.get(3));

        encode.append(String.format(
            "\tif err := _m.Write%1$s(_w, %2$s(len(%3$s.%4$s))); err != nil {\n" +
            "\t\treturn err\n" +
            "\t}\n" +
            "\tif err := _m.WriteBytes(_w, %3$s.%4$s); err != nil {\n" +
            "\t\treturn err\n" +
            "\t}\n",
            golangTypeForLengthMarshal,
            golangTypeForLength,
            varName,
            propertyName));

        decode.append(String.format(
            "\n" +
            "\tif %1$c.%2$sInActingVersion(actingVersion) {\n" +
            "\t\tvar %2$sLength %4$s\n" +
            "\t\tif err := _m.Read%3$s(_r, &%2$sLength); err != nil {\n" +
            "\t\t\treturn err\n" +
            "\t\t}\n" +
            "\t\tif cap(%1$c.%2$s) < int(%2$sLength) {\n" +
            "\t\t\t%1$s.%2$s = make([]%5$s, %2$sLength)\n" +
            "\t\t}\n" +
            "\t\t%1$c.%2$s = %1$c.%2$s[:%2$sLength]\n" +
            "\t\tif err := _m.ReadBytes(_r, %1$c.%2$s); err != nil {\n" +
            "\t\t\treturn err\n" +
            "\t\t}\n" +
            "\t}\n",
            varName,
            propertyName,
            golangTypeForLengthMarshal,
            golangTypeForLength,
            golangTypeForData));

        return gap;
    }

    // returns how much to add to offset
    private int generateGroupEncodeDecode(
        final List<Token> tokens,
        final String typeName,
        final StringBuilder encode,
        final StringBuilder decode,
        final StringBuilder rc,
        final int currentOffset)
    {
        final char varName = Character.toLowerCase(typeName.charAt(0));
        final Token signalToken = tokens.get(0);
        final String propertyName = formatPropertyName(signalToken.name());
        final Token blockLengthToken = Generators.findFirst("blockLength", tokens, 0);
        final Token numInGroupToken = Generators.findFirst("numInGroup", tokens, 0);
        final int blockLengthOffset = blockLengthToken.offset();
        final String blockLengthType = golangTypeName(blockLengthToken.encoding().primitiveType());
        final String blockLengthMarshalType = golangMarshalType(blockLengthToken.encoding().primitiveType());
        final int numInGroupOffset = numInGroupToken.offset();
        final String numInGroupType = golangTypeName(numInGroupToken.encoding().primitiveType());
        final String numInGroupMarshalType = golangMarshalType(numInGroupToken.encoding().primitiveType());

        // Offset handling
        final int gap = Math.max(signalToken.offset() - currentOffset, 0);
        encode.append(generateEncodeOffset(gap, ""));
        decode.append(generateDecodeOffset(gap, ""));

        final String encBlockLengthTmpl =
            "\tvar %7$sBlockLength %1$s = %2$d\n" +
            "\tif err := _m.Write%6$s(_w, %7$sBlockLength); err != nil {\n" +
            "\t\treturn err\n" +
            "\t}\n";

        final String encNumInGroupTmpl =
            "\tvar %7$sNumInGroup %3$s = %3$s(len(%4$s.%5$s))\n" +
            "\tif err := _m.Write%8$s(_w, %7$sNumInGroup); err != nil {\n" +
            "\t\treturn err\n" +
            "\t}\n";

        // Order write based on offset
        final String encGrpMetaTmpl = blockLengthOffset < numInGroupOffset ?
            encBlockLengthTmpl + encNumInGroupTmpl : encNumInGroupTmpl + encBlockLengthTmpl;

        encode.append(String.format(encGrpMetaTmpl,
            blockLengthType,
            signalToken.encodedLength(),
            numInGroupType,
            varName,
            toUpperFirstChar(signalToken.name()),
            blockLengthMarshalType,
            propertyName,
            numInGroupMarshalType));

        // Write the group itself
        encode.append(String.format(
            "\tfor _, prop := range %1$s.%2$s {\n" +
            "\t\tif err := prop.Encode(_m, _w); err != nil {\n" +
            "\t\t\treturn err\n" +
            "\t\t}\n",
            varName,
            toUpperFirstChar(signalToken.name())));

        decode.append(String.format(
            "\n" +
            "\tif %1$s.%2$sInActingVersion(actingVersion) {\n",
            varName,
            propertyName));

        final String decBlockLengthTmpl =
            "\t\tvar %1$sBlockLength %2$s\n" +
            "\t\tif err := _m.Read%4$s(_r, &%1$sBlockLength); err != nil {\n" +
            "\t\t\treturn err\n" +
            "\t\t}\n";

        final String decNumInGroupTmpl =
            "\t\tvar %1$sNumInGroup %3$s\n" +
            "\t\tif err := _m.Read%5$s(_r, &%1$sNumInGroup); err != nil {\n" +
            "\t\t\treturn err\n" +
            "\t\t}\n";

        final String decGrpMetaTmpl = blockLengthOffset < numInGroupOffset ?
            decBlockLengthTmpl + decNumInGroupTmpl : decNumInGroupTmpl + decBlockLengthTmpl;

        decode.append(String.format(decGrpMetaTmpl,
            propertyName,
            blockLengthType,
            numInGroupType,
            blockLengthMarshalType,
            numInGroupMarshalType));

        // Read the group itself
        decode.append(String.format(
            "\t\tif cap(%1$c.%2$s) < int(%2$sNumInGroup) {\n" +
            "\t\t\t%1$s.%2$s = make([]%3$s%2$s, %2$sNumInGroup)\n" +
            "\t\t}\n" +
            "\t\t%1$c.%2$s = %1$c.%2$s[:%2$sNumInGroup]\n" +
            "\t\tfor i := range %1$s.%2$s {\n" +
            "\t\t\tif err := %1$s.%2$s[i].Decode(_m, _r, actingVersion, uint(%4$sBlockLength)); err != nil {\n" +
            "\t\t\t\treturn err\n" +
            "\t\t\t}\n" +
            "\t\t}\n",
            varName,
            toUpperFirstChar(signalToken.name()),
            typeName,
            propertyName));

        // Range check the group itself
        rc.append(String.format(
            "\tfor _, prop := range %1$s.%2$s {\n" +
            "\t\tif err := prop.RangeCheck(actingVersion, schemaVersion); err != nil {\n" +
            "\t\t\treturn err\n" +
            "\t\t}\n" +
            "\t}\n",
            varName,
            toUpperFirstChar(signalToken.name())));

        return gap;
    }

    // Recursively traverse groups to create the group properties
    private void generateGroupProperties(
        final StringBuilder sb,
        final List<Token> tokens,
        final String prefix)
    {
        for (int i = 0, size = tokens.size(); i < size; i++)
        {
            final Token token = tokens.get(i);
            if (token.signal() == Signal.BEGIN_GROUP)
            {
                final String propertyName = formatPropertyName(token.name());

                generateId(sb, prefix, propertyName, token);
                generateSinceActingDeprecated(sb, prefix, propertyName, token);
                generateExtensibilityMethods(sb, prefix + propertyName, token);

                // Look inside for nested groups with extra prefix
                generateGroupProperties(
                    sb,
                    tokens.subList(i + 1, i + token.componentTokenCount() - 1),
                    prefix + propertyName);
                i += token.componentTokenCount() - 1;
            }
        }
    }

    private void generateGroups(final StringBuilder sb, final List<Token> tokens, final String prefix)
    {
        for (int i = 0, size = tokens.size(); i < size; i++)
        {
            final Token groupToken = tokens.get(i);
            if (groupToken.signal() != Signal.BEGIN_GROUP)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_GROUP: token=" + groupToken);
            }

            // Make a unique Group name by adding our parent
            final String groupName = prefix + formatTypeName(groupToken.name());

            ++i;
            final int groupHeaderTokenCount = tokens.get(i).componentTokenCount();
            i += groupHeaderTokenCount;

            final List<Token> fields = new ArrayList<>();
            i = collectFields(tokens, i, fields);
            generateFields(sb, groupName, fields);

            final List<Token> groups = new ArrayList<>();
            i = collectGroups(tokens, i, groups);
            generateGroups(sb, groups, groupName);

            final List<Token> varData = new ArrayList<>();
            i = collectVarData(tokens, i, varData);
            generateVarData(sb, formatTypeName(groupName), varData);
        }
    }

    private void generateVarData(
        final StringBuilder sb,
        final String typeName,
        final List<Token> tokens)
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
            final int lengthOfLengthField = lengthToken.encodedLength();
            final Token varDataToken = Generators.findFirst("varData", tokens, i);
            final String characterEncoding = varDataToken.encoding().characterEncoding();

            generateFieldMetaAttributeMethod(sb, typeName, propertyName, token);
            generateVarDataDescriptors(sb, token, typeName, propertyName, characterEncoding, lengthOfLengthField);

            i += token.componentTokenCount();
        }
    }

    private void generateVarDataDescriptors(
        final StringBuilder sb,
        final Token token,
        final String typeName,
        final String propertyName,
        final String characterEncoding,
        final Integer lengthOfLengthField)
    {
        generateSinceActingDeprecated(sb, typeName, propertyName, token);
        sb.append(String.format(
            "\nfunc (%1$s) %2$sCharacterEncoding() string {\n" +
            "\treturn \"%3$s\"\n" +
            "}\n" +
            "\nfunc (%1$s) %2$sHeaderLength() uint64 {\n" +
            "\treturn %4$s\n" +
            "}\n",
            typeName,
            propertyName,
            characterEncoding,
            lengthOfLengthField));
    }

    private void generateChoiceSet(final List<Token> tokens) throws IOException
    {
        final Token choiceToken = tokens.get(0);
        final String choiceName = formatTypeName(choiceToken.applicableTypeName());
        final StringBuilder sb = new StringBuilder();

        try (Writer out = outputManager.createOutput(choiceName))
        {
            imports.push(new TreeSet<>());
            imports.peek().add("io");

            generateChoiceDecls(
                sb,
                choiceName,
                tokens.subList(1, tokens.size() - 1),
                choiceToken);

            generateChoiceEncodeDecode(sb, choiceName, choiceToken);

            // EncodedLength
            sb.append(String.format(
                "\nfunc (%1$s) EncodedLength() int64 {\n" +
                "\treturn %2$s\n" +
                "}\n",
                choiceName,
                choiceToken.encodedLength()));

            for (final Token token : tokens.subList(1, tokens.size() - 1))
            {
                generateSinceActingDeprecated(sb, choiceName, token.name(), token);
            }
            out.append(generateFileHeader(ir.namespaces()));
            out.append(sb);
            imports.pop();
        }
    }

    private void generateEnum(final List<Token> tokens) throws IOException
    {
        final Token enumToken = tokens.get(0);
        final String enumName = formatTypeName(tokens.get(0).applicableTypeName());

        final StringBuilder sb = new StringBuilder();

        try (Writer out = outputManager.createOutput(enumName))
        {
            imports.push(new TreeSet<>());
            imports.peek().add("io");

            generateEnumDecls(
                sb,
                enumName,
                golangTypeName(tokens.get(0).encoding().primitiveType()),
                tokens.subList(1, tokens.size() - 1),
                enumToken);

            generateEnumEncodeDecode(sb, enumName, enumToken);

            // EncodedLength
            sb.append(String.format(
                "\nfunc (*%1$sEnum) EncodedLength() int64 {\n" +
                "\treturn %2$s\n" +
                "}\n",
                enumName,
                enumToken.encodedLength()));

            for (final Token token : tokens.subList(1, tokens.size() - 1))
            {
                generateSinceActingDeprecated(sb, enumName + "Enum", token.name(), token);
            }

            out.append(generateFileHeader(ir.namespaces()));
            out.append(sb);
            imports.pop();
        }
    }

    private void generateComposite(final List<Token> tokens, final String namePrefix) throws IOException
    {
        final String compositeName = namePrefix + formatTypeName(tokens.get(0).applicableTypeName());
        final StringBuilder sb = new StringBuilder();

        try (Writer out = outputManager.createOutput(compositeName))
        {
            imports.push(new TreeSet<>());
            imports.peek().add("io");

            generateTypeDeclaration(sb, compositeName);
            generateTypeBodyComposite(sb, compositeName, tokens.subList(1, tokens.size() - 1));

            generateEncodeDecode(sb, compositeName, tokens.subList(1, tokens.size() - 1), false, false);
            generateEncodedLength(sb, compositeName, tokens.get(0).encodedLength());

            generateCompositePropertyElements(sb, compositeName, tokens.subList(1, tokens.size() - 1));

            // The FileHeader needs to know which imports to add so
            // it's created last once that's known.
            out.append(generateFileHeader(ir.namespaces()));
            out.append(sb);
            imports.pop();
        }
    }

    private void generateEnumDecls(
        final StringBuilder sb,
        final String enumName,
        final String golangType,
        final List<Token> tokens,
        final Token encodingToken)
    {
        // gofmt lines up the types and we don't want it to have to rewrite
        // our generated files. To line things up we need to know the longest
        // string length and then fill with whitespace
        final String nullValue = "NullValue";
        int longest = nullValue.length();
        for (final Token token : tokens)
        {
            longest = Math.max(longest, token.name().length());
        }

        // Enums are modelled as a struct and we export an instance so
        // you can reference known values as expected.
        sb.append(String.format(
            "type %1$sEnum %2$s\n" +
            "type %1$sValues struct {\n",
            enumName,
            golangType));

        for (final Token token : tokens)
        {
            sb.append(String.format(
                "\t%1$s%2$s%3$sEnum\n",
                token.name(),
                generateWhitespace(longest - token.name().length() + 1),
                enumName));
        }

        // Add the NullValue
        sb.append(String.format(
            "\t%1$s%2$s%3$sEnum\n" +
            "}\n",
            nullValue,
            generateWhitespace(longest - nullValue.length() + 1),
            enumName));

        // And now the Enum Values expressed as a variable
        sb.append(String.format(
            "\nvar %1$s = %1$sValues{",
            enumName));

        for (final Token token : tokens)
        {
            sb.append(generateLiteral(
                token.encoding().primitiveType(), token.encoding().constValue().toString())).append(", ");
        }

        // Add the NullValue and close
        sb.append(encodingToken.encoding().applicableNullValue().toString()).append("}\n");
    }

    private void generateChoiceDecls(
        final StringBuilder sb,
        final String choiceName,
        final List<Token> tokens,
        final Token encodingToken)
    {
        // gofmt lines up the types and we don't want it to have to rewrite
        // our generated files. To line things up we need to know the longest
        // string length and then fill with whitespace
        int longest = 0;
        for (final Token token : tokens)
        {
            longest = Math.max(longest, token.name().length());
        }

        // A ChoiceSet is modelled as an array of bool of size
        // encodedLength in bits (akin to bits in a bitfield).
        // Choice values are modelled as a struct and we export an
        // instance so you can reference known values by name.
        sb.append(String.format(
            "type %1$s [%2$d]bool\n" +
            "type %1$sChoiceValue uint8\n" +
            "type %1$sChoiceValues struct {\n",
            choiceName, encodingToken.encodedLength() * 8));

        for (final Token token : tokens)
        {
            sb.append(String.format(
                "\t%1$s%2$s%3$sChoiceValue\n",
                toUpperFirstChar(token.name()),
                generateWhitespace(longest - token.name().length() + 1),
                toUpperFirstChar(encodingToken.applicableTypeName())));
        }

        sb.append("}\n");

        // And now the Values expressed as a variable
        sb.append(String.format(
            "\nvar %1$sChoice = %1$sChoiceValues{",
            choiceName));

        String comma = "";
        for (final Token token : tokens)
        {
            sb.append(comma)
                .append(generateLiteral(token.encoding().primitiveType(), token.encoding().constValue().toString()));
            comma = ", ";
        }

        sb.append("}\n");
    }

    private String namespacesToPackageName(final CharSequence[] namespaces)
    {
        return String.join("_", namespaces).toLowerCase().replace('.', '_').replace(' ', '_').replace('-', '_');
    }

    private StringBuilder generateFileHeader(final CharSequence[] namespaces)
    {
        final StringBuilder sb = new StringBuilder();
        sb.append("// Generated SBE (Simple Binary Encoding) message codec\n\n");
        sb.append(String.format(
            "package %1$s\n" +
            "\n" +
            "import (\n",
            namespacesToPackageName(namespaces)));

        for (final String s : imports.peek())
        {
            sb.append("\t\"").append(s).append("\"\n");
        }

        sb.append(")\n\n");

        return sb;
    }

    private String generateFromTemplate(final CharSequence[] namespaces, final String templateName)
        throws IOException
    {
        final String templateFileName = "golang/templates/" + templateName + ".go";
        final InputStream stream = getClass().getClassLoader().getResourceAsStream(templateFileName);
        if (null == stream)
        {
            return "";
        }

        try (InputStream in = new BufferedInputStream(stream))
        {
            final Scanner scanner = new Scanner(in).useDelimiter("\\A");
            if (!scanner.hasNext())
            {
                return "";
            }

            return String.format(scanner.next(), namespacesToPackageName(namespaces));
        }
    }

    private static void generateTypeDeclaration(final StringBuilder sb, final String typeName)
    {
        sb.append(String.format("type %s struct {\n", typeName));
    }

    private void generateTypeBody(
        final StringBuilder sb,
        final String typeName,
        final List<Token> tokens)
    {
        // gofmt lines up the types and we don't want it to have to rewrite
        // our generated files. To line things up we need to know the longest
        // string length and then fill with whitespace
        int longest = 0;
        for (int i = 0; i < tokens.size(); i++)
        {
            final Token token = tokens.get(i);
            final String propertyName = formatPropertyName(token.name());

            switch (token.signal())
            {
                case BEGIN_GROUP:
                case BEGIN_VAR_DATA:
                    longest = Math.max(longest, propertyName.length());
                    i += token.componentTokenCount() - 1;
                    break;

                case BEGIN_FIELD:
                    longest = Math.max(longest, propertyName.length());
                    break;

                case END_GROUP:
                    i = tokens.size(); // terminate the loop
                    break;

                default:
                    break;
            }
        }

        final StringBuilder nested = new StringBuilder(); // For nested groups
        for (int i = 0; i < tokens.size(); i++)
        {
            final Token signalToken = tokens.get(i);
            final String propertyName = formatPropertyName(signalToken.name());
            final int length = longest - propertyName.length() + 1;

            switch (signalToken.signal())
            {
                case BEGIN_FIELD:
                    if (tokens.size() > i + 1)
                    {
                        final Token encodingToken = tokens.get(i + 1);

                        // it's an array if length > 1, otherwise normally not
                        String arrayspec = "";
                        if (encodingToken.arrayLength() > 1)
                        {
                            arrayspec = "[" + encodingToken.arrayLength() + "]";
                        }

                        switch (encodingToken.signal())
                        {
                            case BEGIN_ENUM:
                                sb.append("\t").append(propertyName)
                                    .append(generateWhitespace(length))
                                    .append(arrayspec)
                                    .append(encodingToken.applicableTypeName())
                                    .append("Enum\n");
                                break;

                            case BEGIN_SET:
                                sb.append("\t").append(propertyName)
                                    .append(generateWhitespace(length))
                                    .append(arrayspec)
                                    .append(encodingToken.applicableTypeName())
                                    .append("\n");
                                break;

                            default:
                                // If the type is primitive then use the golang naming for it
                                String golangType;
                                golangType = golangTypeName(encodingToken.encoding().primitiveType());
                                if (golangType == null)
                                {
                                    golangType = toUpperFirstChar(encodingToken.name());
                                }
                                // If primitiveType="char" and presence="constant"
                                // then this is actually a character array which
                                // can be of length 1
                                if (encodingToken.isConstantEncoding() &&
                                    encodingToken.encoding().primitiveType() == CHAR)
                                {
                                    arrayspec = "[" + encodingToken.encoding().constValue().size() + "]";
                                }
                                sb.append("\t").append(propertyName)
                                    .append(generateWhitespace(length))
                                    .append(arrayspec).append(golangType).append("\n");
                                break;
                        }
                        i++;
                    }
                    break;

                case BEGIN_GROUP:
                    sb.append(String.format(
                        "\t%1$s%2$s[]%3$s%1$s\n",
                        toUpperFirstChar(signalToken.name()),
                        generateWhitespace(length),
                        typeName));
                    generateTypeDeclaration(
                        nested,
                        typeName + toUpperFirstChar(signalToken.name()));
                    generateTypeBody(
                        nested,
                        typeName + toUpperFirstChar(signalToken.name()),
                        tokens.subList(i + 1, tokens.size() - 1));
                    i += signalToken.componentTokenCount() - 1;
                    break;

                case END_GROUP:
                    // Close the group and unwind
                    sb.append("}\n");
                    sb.append(nested);
                    return;

                case BEGIN_VAR_DATA:
                    sb.append(String.format(
                        "\t%1$s%2$s[]%3$s\n",
                        toUpperFirstChar(signalToken.name()),
                        generateWhitespace(length),
                        golangTypeName(tokens.get(i + 3).encoding().primitiveType())));
                    break;

                default:
                    break;
            }
        }
        sb.append("}\n");
        sb.append(nested);
    }

    private void generateCompositePropertyElements(
        final StringBuilder sb,
        final String containingTypeName,
        final List<Token> tokens)
    {
        for (int i = 0; i < tokens.size(); )
        {
            final Token token = tokens.get(i);
            final String propertyName = formatPropertyName(token.name());

            // Write {Min,Max,Null}Value
            if (token.signal() == Signal.ENCODING)
            {
                generateMinMaxNull(sb, containingTypeName, propertyName, token);
                generateCharacterEncoding(sb, containingTypeName, propertyName, token);
            }

            switch (token.signal())
            {
                case ENCODING:
                case BEGIN_ENUM:
                case BEGIN_SET:
                case BEGIN_COMPOSITE:
                    generateSinceActingDeprecated(sb, containingTypeName, propertyName, token);
                    break;

                default:
                    break;
            }

            i += tokens.get(i).componentTokenCount();
        }
    }

    private void generateMinMaxNull(
        final StringBuilder sb,
        final String typeName,
        final String propertyName,
        final Token token)
    {
        final Encoding encoding = token.encoding();
        final PrimitiveType primitiveType = encoding.primitiveType();
        final String golangTypeName = golangTypeName(primitiveType);
        final CharSequence nullValueString = generateNullValueLiteral(primitiveType, encoding);
        final CharSequence maxValueString = generateMaxValueLiteral(primitiveType, encoding);
        final CharSequence minValueString = generateMinValueLiteral(primitiveType, encoding);

        // MinValue
        sb.append(String.format(
            "\nfunc (*%1$s) %2$sMinValue() %3$s {\n" +
            "\treturn %4$s\n" +
            "}\n",
            typeName,
            propertyName,
            golangTypeName,
            minValueString));

        // MaxValue
        sb.append(String.format(
            "\nfunc (*%1$s) %2$sMaxValue() %3$s {\n" +
            "\treturn %4$s\n" +
            "}\n",
            typeName,
            propertyName,
            golangTypeName,
            maxValueString));

        // NullValue
        sb.append(String.format(
            "\nfunc (*%1$s) %2$sNullValue() %3$s {\n" +
            "\treturn %4$s\n" +
            "}\n",
            typeName,
            propertyName,
            golangTypeName,
            nullValueString));
    }

    private void generateCharacterEncoding(
        final StringBuilder sb,
        final String typeName,
        final String propertyName,
        final Token token)
    {
        if (token.encoding().primitiveType() == CHAR && token.arrayLength() > 1)
        {
            sb.append(String.format(
                "\nfunc (%1$s *%2$s) %3$sCharacterEncoding() string {\n" +
                "\treturn \"%4$s\"\n" +
                "}\n",
                Character.toLowerCase(typeName.charAt(0)),
                typeName,
                propertyName,
                token.encoding().characterEncoding()));
        }
    }

    private void generateId(
        final StringBuilder sb,
        final String typeName,
        final String propertyName,
        final Token token)
    {
        sb.append(String.format(
            "\nfunc (*%1$s) %2$sId() uint16 {\n" +
            "\treturn %3$s\n" +
            "}\n",
            typeName,
            propertyName,
            token.id()));
    }

    private void generateSinceActingDeprecated(
        final StringBuilder sb,
        final String typeName,
        final String propertyName,
        final Token token)
    {
        sb.append(String.format(
            "\nfunc (*%2$s) %3$sSinceVersion() uint16 {\n" +
            "\treturn %4$s\n" +
            "}\n" +
            "\nfunc (%1$s *%2$s) %3$sInActingVersion(actingVersion uint16) bool {\n" +
            "\treturn actingVersion >= %1$s.%3$sSinceVersion()\n" +
            "}\n" +
            "\nfunc (*%2$s) %3$sDeprecated() uint16 {\n" +
            "\treturn %5$s\n" +
            "}\n",
            Character.toLowerCase(typeName.charAt(0)),
            typeName,
            propertyName,
            token.version(),
            token.deprecated()));
    }

    private void generateTypeBodyComposite(
        final StringBuilder sb,
        final String typeName,
        final List<Token> tokens) throws IOException
    {
        // gofmt lines up the types, and we don't want it to have to rewrite
        // our generated files. To line things up we need to know the longest
        // string length and then fill with whitespace
        int longest = 0;
        for (int i = 0; i < tokens.size(); i++)
        {
            final Token token = tokens.get(i);
            final String propertyName = formatPropertyName(token.name());

            switch (token.signal())
            {
                case BEGIN_GROUP:
                case BEGIN_COMPOSITE:
                case BEGIN_VAR_DATA:
                    longest = Math.max(longest, propertyName.length());
                    i += token.componentTokenCount() - 1;
                    break;

                case BEGIN_ENUM:
                case BEGIN_SET:
                case ENCODING:
                    longest = Math.max(longest, propertyName.length());
                    break;

                case END_COMPOSITE:
                    i = tokens.size(); // terminate the loop
                    break;

                default:
                    break;
            }
        }

        for (int i = 0; i < tokens.size(); i++)
        {
            final Token token = tokens.get(i);
            final String propertyName = formatPropertyName(token.name());
            final String propertyType = formatPropertyName(token.applicableTypeName());
            int arrayLength = token.arrayLength();

            switch (token.signal())
            {
                case ENCODING:
                    // if a primitiveType="char" and presence="constant" then this is actually a character array
                    if (token.isConstantEncoding() && token.encoding().primitiveType() == CHAR)
                    {
                        arrayLength = token.encoding().constValue().size(); // can be 1
                        sb.append("\t").append(propertyName)
                            .append(generateWhitespace(longest - propertyName.length() + 1))
                            .append("[").append(arrayLength).append("]")
                            .append(golangTypeName(token.encoding().primitiveType())).append("\n");
                    }
                    else
                    {
                        sb.append("\t").append(propertyName)
                            .append(generateWhitespace(longest - propertyName.length() + 1))
                            .append((arrayLength > 1) ? ("[" + arrayLength + "]") : "")
                            .append(golangTypeName(token.encoding().primitiveType())).append("\n");
                    }
                    break;

                case BEGIN_ENUM:
                    sb.append("\t").append(propertyName)
                        .append(generateWhitespace(longest - propertyName.length() + 1))
                        .append((arrayLength > 1) ? ("[" + arrayLength + "]") : "")
                        .append(propertyType).append("Enum\n");
                    break;

                case BEGIN_SET:
                    sb.append("\t").append(propertyName)
                        .append(generateWhitespace(longest - propertyName.length() + 1))
                        .append((arrayLength > 1) ? ("[" + arrayLength + "]") : "").append(propertyType).append("\n");
                    break;

                case BEGIN_COMPOSITE:
                    // recurse
                    generateComposite(tokens.subList(i, i + token.componentTokenCount()), typeName);
                    i += token.componentTokenCount() - 1;

                    sb.append("\t").append(propertyName)
                        .append(generateWhitespace(longest - propertyName.length() + 1))
                        .append((arrayLength > 1) ? ("[" + arrayLength + "]") : "")
                        .append(typeName).append(propertyType).append("\n");
                    break;

                default:
                    break;
            }
        }
        sb.append("}\n");
    }

    private void generateEncodedLength(
        final StringBuilder sb,
        final String typeName,
        final int size)
    {
        sb.append(String.format(
            "\nfunc (*%1$s) EncodedLength() int64 {\n" +
            "\treturn %2$s\n" +
            "}\n",
            typeName,
            size));
    }

    private void generateMessageCode(
        final StringBuilder sb,
        final String typeName,
        final List<Token> tokens)
    {
        final Token token = tokens.get(0);
        final String semanticType = token.encoding().semanticType() == null ? "" : token.encoding().semanticType();
        final String blockLengthType = golangTypeName(ir.headerStructure().blockLengthType());
        final String templateIdType = golangTypeName(ir.headerStructure().templateIdType());
        final String schemaIdType = golangTypeName(ir.headerStructure().schemaIdType());
        final String schemaVersionType = golangTypeName(ir.headerStructure().schemaVersionType());
        final String semanticVersion = ir.semanticVersion() == null ? "" : ir.semanticVersion();

        generateEncodeDecode(sb, typeName, tokens, true, true);

        sb.append(String.format(
            "\nfunc (*%1$s) SbeBlockLength() (blockLength %2$s) {\n" +
            "\treturn %3$s\n" +
            "}\n" +
            "\nfunc (*%1$s) SbeTemplateId() (templateId %4$s) {\n" +
            "\treturn %5$s\n" +
            "}\n" +
            "\nfunc (*%1$s) SbeSchemaId() (schemaId %6$s) {\n" +
            "\treturn %7$s\n" +
            "}\n" +
            "\nfunc (*%1$s) SbeSchemaVersion() (schemaVersion %8$s) {\n" +
            "\treturn %9$s\n" +
            "}\n" +
            "\nfunc (*%1$s) SbeSemanticType() (semanticType []byte) {\n" +
            "\treturn []byte(\"%10$s\")\n" +
            "}\n" +
            "\nfunc (*%1$s) SbeSemanticVersion() (semanticVersion string) {\n" +
            "\treturn \"%11$s\"\n" +
            "}\n",
            typeName,
            blockLengthType,
            generateLiteral(ir.headerStructure().blockLengthType(), Integer.toString(token.encodedLength())),
            templateIdType,
            generateLiteral(ir.headerStructure().templateIdType(), Integer.toString(token.id())),
            schemaIdType,
            generateLiteral(ir.headerStructure().schemaIdType(), Integer.toString(ir.id())),
            schemaVersionType,
            generateLiteral(ir.headerStructure().schemaVersionType(), Integer.toString(ir.version())),
            semanticType,
            semanticVersion));
    }

    // Used for groups which need to know the schema's definition
    // of block length and version to check for extensions
    private void generateExtensibilityMethods(
        final StringBuilder sb,
        final String typeName,
        final Token token)
    {
        sb.append(String.format(
            "\nfunc (*%1$s) SbeBlockLength() (blockLength uint) {\n" +
            "\treturn %2$s\n" +
            "}\n" +
            "\nfunc (*%1$s) SbeSchemaVersion() (schemaVersion %3$s) {\n" +
            "\treturn %4$s\n" +
            "}\n",
            typeName,
            generateLiteral(ir.headerStructure().blockLengthType(), Integer.toString(token.encodedLength())),
            golangTypeName(ir.headerStructure().schemaVersionType()),
            generateLiteral(ir.headerStructure().schemaVersionType(), Integer.toString(ir.version()))));
    }

    private void generateFields(
        final StringBuilder sb,
        final String containingTypeName,
        final List<Token> tokens)
    {
        for (int i = 0, size = tokens.size(); i < size; i++)
        {
            final Token signalToken = tokens.get(i);
            if (signalToken.signal() == Signal.BEGIN_FIELD)
            {
                final Token encodingToken = tokens.get(i + 1);
                final String propertyName = formatPropertyName(signalToken.name());

                generateId(sb, containingTypeName, propertyName, signalToken);
                generateSinceActingDeprecated(sb, containingTypeName, propertyName, signalToken);
                generateFieldMetaAttributeMethod(sb, containingTypeName, propertyName, signalToken);

                if (encodingToken.signal() == Signal.ENCODING)
                {
                    generateMinMaxNull(sb, containingTypeName, propertyName, encodingToken);
                    generateCharacterEncoding(sb, containingTypeName, propertyName, encodingToken);
                }
            }
        }
    }

    private static void generateFieldMetaAttributeMethod(
        final StringBuilder sb,
        final String containingTypeName,
        final String propertyName,
        final Token token)
    {
        final Encoding encoding = token.encoding();
        final String epoch = encoding.epoch() == null ? "" : encoding.epoch();
        final String timeUnit = encoding.timeUnit() == null ? "" : encoding.timeUnit();
        final String semanticType = encoding.semanticType() == null ? "" : encoding.semanticType();
        final String presence = encoding.presence() == null ? "" : encoding.presence().toString().toLowerCase();

        sb.append(String.format(
            "\nfunc (*%1$s) %2$sMetaAttribute(meta int) string {\n" +
            "\tswitch meta {\n" +
            "\tcase 1:\n" +
            "\t\treturn \"%3$s\"\n" +
            "\tcase 2:\n" +
            "\t\treturn \"%4$s\"\n" +
            "\tcase 3:\n" +
            "\t\treturn \"%5$s\"\n" +
            "\tcase 4:\n" +
            "\t\treturn \"%6$s\"\n" +
            "\t}\n" +
            "\treturn \"\"\n" +
            "}\n",
            containingTypeName,
            propertyName,
            epoch,
            timeUnit,
            semanticType,
            presence));
    }

    private CharSequence generateMinValueLiteral(final PrimitiveType primitiveType, final Encoding encoding)
    {
        if (null == encoding.maxValue())
        {
            switch (primitiveType)
            {
                case CHAR:
                    return "byte(32)";
                case INT8:
                    imports.peek().add("math");
                    return "math.MinInt8 + 1";
                case INT16:
                    imports.peek().add("math");
                    return "math.MinInt16 + 1";
                case INT32:
                    imports.peek().add("math");
                    return "math.MinInt32 + 1";
                case INT64:
                    imports.peek().add("math");
                    return "math.MinInt64 + 1";
                case UINT8:
                case UINT16:
                case UINT32:
                case UINT64:
                    return "0";
                case FLOAT:
                    imports.peek().add("math");
                    return "-math.MaxFloat32";
                case DOUBLE:
                    imports.peek().add("math");
                    return "-math.MaxFloat64";
            }
        }

        return generateLiteral(primitiveType, encoding.applicableMinValue().toString());
    }

    private CharSequence generateMaxValueLiteral(final PrimitiveType primitiveType, final Encoding encoding)
    {
        if (null == encoding.maxValue())
        {
            switch (primitiveType)
            {
                case CHAR:
                    return "byte(126)";
                case INT8:
                    imports.peek().add("math");
                    return "math.MaxInt8";
                case INT16:
                    imports.peek().add("math");
                    return "math.MaxInt16";
                case INT32:
                    imports.peek().add("math");
                    return "math.MaxInt32";
                case INT64:
                    imports.peek().add("math");
                    return "math.MaxInt64";
                case UINT8:
                    imports.peek().add("math");
                    return "math.MaxUint8 - 1";
                case UINT16:
                    imports.peek().add("math");
                    return "math.MaxUint16 - 1";
                case UINT32:
                    imports.peek().add("math");
                    return "math.MaxUint32 - 1";
                case UINT64:
                    imports.peek().add("math");
                    return "math.MaxUint64 - 1";
                case FLOAT:
                    imports.peek().add("math");
                    return "math.MaxFloat32";
                case DOUBLE:
                    imports.peek().add("math");
                    return "math.MaxFloat64";
            }
        }

        return generateLiteral(primitiveType, encoding.applicableMaxValue().toString());
    }

    private CharSequence generateNullValueLiteral(final PrimitiveType primitiveType, final Encoding encoding)
    {
        if (null == encoding.nullValue())
        {
            switch (primitiveType)
            {
                case INT8:
                    imports.peek().add("math");
                    return "math.MinInt8";
                case INT16:
                    imports.peek().add("math");
                    return "math.MinInt16";
                case INT32:
                    imports.peek().add("math");
                    return "math.MinInt32";
                case INT64:
                    imports.peek().add("math");
                    return "math.MinInt64";
                case UINT8:
                    imports.peek().add("math");
                    return "math.MaxUint8";
                case UINT16:
                    imports.peek().add("math");
                    return "math.MaxUint16";
                case UINT32:
                    imports.peek().add("math");
                    return "math.MaxUint32";
                case UINT64:
                    imports.peek().add("math");
                    return "math.MaxUint64";
                default:
                    break;
            }
        }

        return generateLiteral(primitiveType, encoding.applicableNullValue().toString());
    }

    private CharSequence generateLiteral(final PrimitiveType type, final String value)
    {
        String literal = "";

        final String castType = golangTypeName(type);
        switch (type)
        {
            case CHAR:
            case INT8:
            case INT16:
            case INT32:
            case UINT8:
            case UINT16:
            case UINT32:
                literal = value;
                break;

            case UINT64:
                // We get negative numbers from the IR as java has
                // signed types only.
                if (value.charAt(0) == '-')
                {
                    literal = Long.toUnsignedString(Long.parseLong(value));
                }
                else
                {
                    literal = castType + "(" + value + ")";
                }
                break;

            case INT64:
                literal = castType + "(" + value + ")";
                break;

            case FLOAT:
                literal = "float32(" + (value.endsWith("NaN") ? "math.NaN()" : value) + ")";
                break;

            case DOUBLE:
                literal = value.endsWith("NaN") ? "math.NaN()" : value;
                break;
        }

        return literal;
    }

    // Always generates at least one space
    private String generateWhitespace(final int spaces)
    {
        final int limitedSpaces = Math.max(1, spaces);
        return String.format(String.format("%%%ds", limitedSpaces), " ");
    }
}

