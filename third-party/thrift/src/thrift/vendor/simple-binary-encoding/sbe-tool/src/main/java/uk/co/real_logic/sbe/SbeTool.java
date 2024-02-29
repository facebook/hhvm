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
package uk.co.real_logic.sbe;

import org.agrona.DirectBuffer;
import org.agrona.MutableDirectBuffer;
import org.xml.sax.InputSource;
import uk.co.real_logic.sbe.generation.CodeGenerator;
import uk.co.real_logic.sbe.generation.TargetCodeGenerator;
import uk.co.real_logic.sbe.generation.TargetCodeGeneratorLoader;
import uk.co.real_logic.sbe.ir.Ir;
import uk.co.real_logic.sbe.ir.IrDecoder;
import uk.co.real_logic.sbe.ir.IrEncoder;
import uk.co.real_logic.sbe.xml.*;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

/**
 * A tool for running the SBE parser, validator, and code generator.
 * <p>
 * Usage:
 * <pre>
 *     $ java -jar sbe.jar &lt;filename.xml&gt;
 *     $ java -Doption=value -jar sbe.jar &lt;filename.xml&gt;
 *     $ java -Doption=value -jar sbe.jar &lt;filename.sbeir&gt;
 * </pre>
 * <p>
 * System Properties:
 * <ul>
 * <li><b>sbe.validation.xsd</b>: Use XSD to validate or not.</li>
 * <li><b>sbe.validation.stop.on.error</b>: Should the parser stop on first error encountered? Defaults to false.</li>
 * <li><b>sbe.validation.warnings.fatal</b>: Are warnings in parsing considered fatal? Defaults to false.</li>
 * <li>
 *     <b>sbe.validation.suppress.output</b>: Should the parser suppress output during validation? Defaults to false.
 * </li>
 * <li><b>sbe.generate.stubs</b>: Generate stubs or not. Defaults to true.</li>
 * <li><b>sbe.target.language</b>: Target language for code generation, defaults to Java.</li>
 * <li><b>sbe.generate.ir</b>: Generate IR or not. Defaults to false.</li>
 * <li><b>sbe.output.dir</b>: Target directory for code generation, defaults to current directory.</li>
 * <li><b>sbe.java.generate.interfaces</b>: Generate interface hierarchy or not. Defaults to false.</li>
 * <li><b>sbe.java.encoding.buffer.type</b>: Type of the Java interface for the encoding buffer to wrap.</li>
 * <li><b>sbe.java.decoding.buffer.type</b>: Type of the Java interface for the decoding buffer to wrap.</li>
 * <li><b>sbe.target.namespace</b>: Namespace for the generated code to override schema package.</li>
 * <li><b>sbe.cpp.namespaces.collapse</b>: Namespace for the generated code to override schema package.</li>
 * <li>
 *     <b>sbe.java.generate.group-order.annotation</b>: Should the GroupOrder annotation be added to generated stubs.
 * </li>
 * <li><b>sbe.csharp.generate.namespace.dir</b>: Should a directory be created for the namespace under
 * the output directory? Defaults to true</li>
 * <li><b>sbe.keyword.append.token</b>: Token to be appended to keywords.</li>
 * <li><b>sbe.decode.unknown.enum.values</b>: Support unknown decoded enum values. Defaults to false.</li>
 * <li><b>sbe.xinclude.aware</b>: Is XInclude supported for the schema. Defaults to false.</li>
 * <li><b>sbe.type.package.override</b>: Is package attribute for types element supported (only for JAVA). Defaults to
 * false.</li>
 * </ul>
 */
public class SbeTool
{
    /**
     * Package in which the generated Java interfaces will be placed.
     */
    public static final String JAVA_INTERFACE_PACKAGE = "org.agrona.sbe";

    /**
     * Default class to use as the buffer mutable implementation in generated code.
     */
    public static final String JAVA_DEFAULT_ENCODING_BUFFER_TYPE = MutableDirectBuffer.class.getName();

    /**
     * Default class to use as the buffer read only implementation in generated code.
     */
    public static final String JAVA_DEFAULT_DECODING_BUFFER_TYPE = DirectBuffer.class.getName();

    /**
     * Boolean system property to control throwing exceptions on all errors.
     */
    public static final String VALIDATION_STOP_ON_ERROR = "sbe.validation.stop.on.error";

    /**
     * Boolean system property to control whether to consider warnings fatal and treat them as errors.
     */
    public static final String VALIDATION_WARNINGS_FATAL = "sbe.validation.warnings.fatal";

    /**
     * System property to hold XSD to validate message specification against.
     */
    public static final String VALIDATION_XSD = "sbe.validation.xsd";

    /**
     * Boolean system property to control suppressing output on all errors and warnings.
     */
    public static final String VALIDATION_SUPPRESS_OUTPUT = "sbe.validation.suppress.output";

    /**
     * Boolean system property to turn on or off generation of stubs. Defaults to true.
     */
    public static final String GENERATE_STUBS = "sbe.generate.stubs";

    /**
     * Boolean system property to control is XInclude is supported. Defaults to false.
     */
    public static final String XINCLUDE_AWARE = "sbe.xinclude.aware";

    /**
     * Boolean system property to control the support of package names in {@code <types>} elements.
     * Part of SBE v2-rc3. Defaults to false.
     */
    public static final String TYPES_PACKAGE_OVERRIDE = "sbe.types.package.override";

    /**
     * Target language for generated code.
     */
    public static final String TARGET_LANGUAGE = "sbe.target.language";

    /**
     * Boolean system property to turn on or off generation of IR. Defaults to false.
     */
    public static final String GENERATE_IR = "sbe.generate.ir";

    /**
     * Output directory for generated code.
     */
    public static final String OUTPUT_DIR = "sbe.output.dir";

    /**
     * String system property of the namespace for generated code.
     */
    public static final String TARGET_NAMESPACE = "sbe.target.namespace";

    /**
     * Boolean system property to toggle collapsing of nested namespaces in generated C++ stubs. Defaults to false.
     */
    public static final String CPP_NAMESPACES_COLLAPSE = "sbe.cpp.namespaces.collapse";

    /**
     * Boolean system property to turn on or off generation of the interface hierarchy. Defaults to false.
     */
    public static final String JAVA_GENERATE_INTERFACES = "sbe.java.generate.interfaces";

    /**
     * Specifies the name of the Java mutable buffer to wrap.
     */
    public static final String JAVA_ENCODING_BUFFER_TYPE = "sbe.java.encoding.buffer.type";

    /**
     * Specifies the name of the Java read only buffer to wrap.
     */
    public static final String JAVA_DECODING_BUFFER_TYPE = "sbe.java.decoding.buffer.type";

    /**
     * Should the {@link uk.co.real_logic.sbe.codec.java.GroupOrder} annotation be added to generated stubs.
     */
    public static final String JAVA_GROUP_ORDER_ANNOTATION = "sbe.java.generate.group-order.annotation";

    /**
     * Boolean system property to turn on or off generation of namespace directories during csharp code generation.
     * Defaults to true
     */
    public static final String CSHARP_GENERATE_NAMESPACE_DIR = "sbe.csharp.generate.namespace.dir";

    /**
     * Specifies token that should be appended to keywords to avoid compilation errors.
     * <p>
     * If none is supplied then use of keywords results in an error during schema parsing. The
     * underscore character is a good example of a token to use.
     */
    public static final String KEYWORD_APPEND_TOKEN = "sbe.keyword.append.token";

    /**
     * Should unknown enum values be decoded to support extension. Defaults to false.
     * <p>
     * If an unknown enum value is decoded then a language specific SBE_UNKNOWN enum value will be returned
     * rather than throwing an error.
     */
    public static final String DECODE_UNKNOWN_ENUM_VALUES = "sbe.decode.unknown.enum.values";

    /**
     * Configuration option used to manage sinceVersion based transformations. When set, parsed schemas will be
     * transformed to discard messages and types higher than the specified version. This can be useful when needing
     * to generate older versions of a schema to do version compatibility testing.
     * <p>
     * This field can contain a list of ordered pairs in the form:
     * <code>((&lt;schema id&gt; | '*') ':' &lt;schema id&gt;)(',' ((&lt;schema id&gt; | '*') ':' &lt;schema id&gt;))*
     * </code>.
     * E.g. <code>123:5,*:6</code> which means transform schema with id = 123 to version 5, all others to version 6.
     */
    public static final String SCHEMA_TRANSFORM_VERSION = "sbe.schema.transform.version";

    /**
     * Whether to generate field precedence checks. For example, whether to check that repeating groups are encoded
     * in schema order.
     */
    public static final String GENERATE_PRECEDENCE_CHECKS = "sbe.generate.precedence.checks";

    /**
     * The name of the symbol or macro that enables access order checks when building
     * generated C# or C++ code.
     */
    public static final String PRECEDENCE_CHECKS_FLAG_NAME = "sbe.precedence.checks.flag.name";

    /**
     * The name of the system property that enables access order checks at runtime
     * in generated Java code.
     */
    public static final String JAVA_PRECEDENCE_CHECKS_PROPERTY_NAME = "sbe.java.precedence.checks.property.name";

    /**
     * Main entry point for the SBE Tool.
     *
     * @param args command line arguments. A single filename is expected.
     * @throws Exception if an error occurs during process of the message schema.
     */
    public static void main(final String[] args) throws Exception
    {
        if (args.length == 0)
        {
            System.err.format("Usage: %s <filenames>...%n", SbeTool.class.getName());
            System.exit(-1);
        }

        for (final String fileName : args)
        {
            final Ir ir;
            if (fileName.endsWith(".xml"))
            {
                final String xsdFilename = System.getProperty(SbeTool.VALIDATION_XSD);
                if (xsdFilename != null)
                {
                    validateAgainstSchema(fileName, xsdFilename);
                }

                final MessageSchema schema = parseSchema(fileName);
                final SchemaTransformer transformer = new SchemaTransformerFactory(
                    System.getProperty(SCHEMA_TRANSFORM_VERSION));
                ir = new IrGenerator().generate(transformer.transform(schema), System.getProperty(TARGET_NAMESPACE));
            }
            else if (fileName.endsWith(".sbeir"))
            {
                try (IrDecoder irDecoder = new IrDecoder(fileName))
                {
                    ir = irDecoder.decode();
                }
            }
            else
            {
                System.err.println("Input file format not supported: " + fileName);
                System.exit(-1);
                return;
            }

            final String outputDirName = System.getProperty(OUTPUT_DIR, ".");
            if (Boolean.parseBoolean(System.getProperty(GENERATE_STUBS, "true")))
            {
                final String targetLanguage = System.getProperty(TARGET_LANGUAGE, "Java");

                generate(ir, outputDirName, targetLanguage);
            }

            if (Boolean.parseBoolean(System.getProperty(GENERATE_IR, "false")))
            {
                final File inputFile = new File(fileName);
                final String inputFilename = inputFile.getName();
                final int nameEnd = inputFilename.lastIndexOf('.');
                final String namePart = inputFilename.substring(0, nameEnd);
                final File fullPath = new File(outputDirName, namePart + ".sbeir");

                try (IrEncoder irEncoder = new IrEncoder(fullPath.getAbsolutePath(), ir))
                {
                    irEncoder.encode();
                }
            }
        }
    }

    /**
     * Validate the SBE Schema against the XSD.
     *
     * @param sbeSchemaFilename to be validated.
     * @param xsdFilename       XSD against which to validate.
     * @throws Exception if an error occurs while validating.
     */
    public static void validateAgainstSchema(final String sbeSchemaFilename, final String xsdFilename)
        throws Exception
    {
        final ParserOptions.Builder optionsBuilder = ParserOptions.builder()
            .xsdFilename(System.getProperty(VALIDATION_XSD))
            .xIncludeAware(Boolean.parseBoolean(System.getProperty(XINCLUDE_AWARE)))
            .stopOnError(Boolean.parseBoolean(System.getProperty(VALIDATION_STOP_ON_ERROR)))
            .warningsFatal(Boolean.parseBoolean(System.getProperty(VALIDATION_WARNINGS_FATAL)))
            .suppressOutput(Boolean.parseBoolean(System.getProperty(VALIDATION_SUPPRESS_OUTPUT)));

        final Path path = Paths.get(sbeSchemaFilename);
        try (InputStream in = new BufferedInputStream(Files.newInputStream(path)))
        {
            final InputSource inputSource = new InputSource(in);
            if (path.toAbsolutePath().getParent() != null)
            {
                inputSource.setSystemId(path.toUri().toString());
            }

            XmlSchemaParser.validate(xsdFilename, inputSource, optionsBuilder.build());
        }
    }

    /**
     * Parse the message schema specification.
     *
     * @param sbeSchemaFilename file containing the SBE specification to be parsed.
     * @return the parsed {@link MessageSchema} for the specification found in the file.
     * @throws Exception if an error occurs when parsing the specification.
     */
    public static MessageSchema parseSchema(final String sbeSchemaFilename)
        throws Exception
    {
        final ParserOptions.Builder optionsBuilder = ParserOptions.builder()
            .xsdFilename(System.getProperty(VALIDATION_XSD))
            .xIncludeAware(Boolean.parseBoolean(System.getProperty(XINCLUDE_AWARE)))
            .stopOnError(Boolean.parseBoolean(System.getProperty(VALIDATION_STOP_ON_ERROR)))
            .warningsFatal(Boolean.parseBoolean(System.getProperty(VALIDATION_WARNINGS_FATAL)))
            .suppressOutput(Boolean.parseBoolean(System.getProperty(VALIDATION_SUPPRESS_OUTPUT)));

        final Path path = Paths.get(sbeSchemaFilename);
        try (InputStream in = new BufferedInputStream(Files.newInputStream(path)))
        {
            final InputSource inputSource = new InputSource(in);
            if (path.toAbsolutePath().getParent() != null)
            {
                inputSource.setSystemId(path.toUri().toString());
            }

            return XmlSchemaParser.parse(inputSource, optionsBuilder.build());
        }
    }

    /**
     * Generate SBE encoding and decoding stubs for a target language.
     *
     * @param ir             for the parsed specification.
     * @param outputDirName  directory into which code will be generated.
     * @param targetLanguage for the generated code.
     * @throws Exception if an error occurs while generating the code.
     */
    public static void generate(final Ir ir, final String outputDirName, final String targetLanguage)
        throws Exception
    {
        final TargetCodeGenerator targetCodeGenerator = TargetCodeGeneratorLoader.get(targetLanguage);
        final CodeGenerator codeGenerator = targetCodeGenerator.newInstance(ir, outputDirName);

        codeGenerator.generate();
    }
}
