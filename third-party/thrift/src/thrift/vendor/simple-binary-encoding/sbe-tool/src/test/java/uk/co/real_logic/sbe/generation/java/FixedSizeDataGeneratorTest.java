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

import org.agrona.DirectBuffer;
import org.agrona.MutableDirectBuffer;
import org.agrona.concurrent.UnsafeBuffer;
import org.agrona.generation.CompilerUtil;
import org.agrona.generation.StringWriterOutputManager;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import uk.co.real_logic.sbe.Tests;
import uk.co.real_logic.sbe.ir.Ir;
import uk.co.real_logic.sbe.xml.IrGenerator;
import uk.co.real_logic.sbe.xml.MessageSchema;
import uk.co.real_logic.sbe.xml.ParserOptions;
import uk.co.real_logic.sbe.xml.XmlSchemaParser;

import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.Map;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.is;
import static uk.co.real_logic.sbe.generation.java.ReflectionUtil.*;

class FixedSizeDataGeneratorTest
{
    private static final Class<?> BUFFER_CLASS = MutableDirectBuffer.class;
    private static final String BUFFER_NAME = BUFFER_CLASS.getName();
    private static final Class<DirectBuffer> READ_ONLY_BUFFER_CLASS = DirectBuffer.class;
    private static final String READ_ONLY_BUFFER_NAME = READ_ONLY_BUFFER_CLASS.getName();

    private final StringWriterOutputManager outputManager = new StringWriterOutputManager();

    private Ir ir;

    @BeforeEach
    void setUp() throws Exception
    {
        try (InputStream in = Tests.getLocalResource("extension-schema.xml"))
        {
            final ParserOptions options = ParserOptions.builder().stopOnError(true).build();
            final MessageSchema schema = XmlSchemaParser.parse(in, options);
            final IrGenerator irg = new IrGenerator();
            ir = irg.generate(schema);

            outputManager.clear();
            outputManager.setPackageName(ir.applicableNamespace());

            generator().generate();
        }
    }

    @Test
    void shouldGeneratePutAndGetByteArrayForFixedLengthBlob() throws Exception
    {
        final UnsafeBuffer buffer = new UnsafeBuffer(new byte[4096]);

        final Object encoder = getTestMessage2Encoder(buffer);
        final Object decoder = getTestMessage2Decoder(buffer, encoder);

        final byte[] encodedData = "  **DATA**  ".getBytes(StandardCharsets.US_ASCII);
        byte[] decodedData;
        int decodedDataLength;

        // Every byte written, every byte read
        putByteArray(encoder, "putTag6", encodedData, 2, 8);
        decodedData = "            ".getBytes(StandardCharsets.US_ASCII);
        decodedDataLength = getByteArray(decoder, "getTag6", decodedData, 1, 8);
        assertThat(decodedDataLength, is(8));
        assertThat(new String(decodedData, StandardCharsets.US_ASCII), is(" **DATA**   "));

        // Every byte written, less bytes read
        putByteArray(encoder, "putTag6", encodedData, 2, 8);
        decodedData = "            ".getBytes(StandardCharsets.US_ASCII);
        decodedDataLength = getByteArray(decoder, "getTag6", decodedData, 1, 6);
        assertThat(decodedDataLength, is(6));
        assertThat(new String(decodedData, StandardCharsets.US_ASCII), is(" **DATA     "));

        // Less bytes written (padding), every byte read
        putByteArray(encoder, "putTag6", encodedData, 2, 6);
        decodedData = "            ".getBytes(StandardCharsets.US_ASCII);
        decodedDataLength = getByteArray(decoder, "getTag6", decodedData, 1, 8);
        assertThat(decodedDataLength, is(8));
        assertThat(new String(decodedData, StandardCharsets.US_ASCII), is(" **DATA\u0000\u0000   "));
    }

    @Test
    void shouldGeneratePutAndGetDirectBufferForFixedLengthBlob() throws Exception
    {
        final UnsafeBuffer buffer = new UnsafeBuffer(new byte[4096]);

        final Object encoder = getTestMessage2Encoder(buffer);
        final Object decoder = getTestMessage2Decoder(buffer, encoder);

        final UnsafeBuffer encodedData = new UnsafeBuffer("  **DATA**  ".getBytes(StandardCharsets.US_ASCII));
        UnsafeBuffer decodedData;
        int decodedDataLength;

        // Every byte written, every byte read
        putDirectBuffer(encoder, "putTag6", encodedData, 2, 8);
        decodedData = new UnsafeBuffer("            ".getBytes(StandardCharsets.US_ASCII));
        decodedDataLength = getDirectBuffer(decoder, "getTag6", decodedData, 1, 8);
        assertThat(decodedDataLength, is(8));
        assertThat(decodedData.getStringWithoutLengthAscii(0, 12), is(" **DATA**   "));

        // Every byte written, less bytes read
        putDirectBuffer(encoder, "putTag6", encodedData, 2, 8);
        decodedData = new UnsafeBuffer("            ".getBytes(StandardCharsets.US_ASCII));
        decodedDataLength = getDirectBuffer(decoder, "getTag6", decodedData, 1, 6);
        assertThat(decodedDataLength, is(6));
        assertThat(decodedData.getStringWithoutLengthAscii(0, 12), is(" **DATA     "));

        // Less bytes written (padding), every byte read
        putDirectBuffer(encoder, "putTag6", encodedData, 2, 6);
        decodedData = new UnsafeBuffer("            ".getBytes(StandardCharsets.US_ASCII));
        decodedDataLength = getDirectBuffer(decoder, "getTag6", decodedData, 1, 8);
        assertThat(decodedDataLength, is(8));
        assertThat(decodedData.getStringWithoutLengthAscii(0, 12), is(" **DATA\u0000\u0000   "));
    }

    @Test
    void shouldGenerateWrapForFixedLengthBlob() throws Exception
    {
        final UnsafeBuffer buffer = new UnsafeBuffer(new byte[4096]);

        final Object encoder = getTestMessage2Encoder(buffer);
        final Object decoder = getTestMessage2Decoder(buffer, encoder);

        final UnsafeBuffer encodedData = new UnsafeBuffer("  **DATA**  ".getBytes(StandardCharsets.US_ASCII));
        putDirectBuffer(encoder, "putTag6", encodedData, 2, 8);

        final UnsafeBuffer decodedData = new UnsafeBuffer();
        wrapDirectBuffer(decoder, "wrapTag6", decodedData);
        assertThat(decodedData.getStringWithoutLengthAscii(0, decodedData.capacity()), is("**DATA**"));
    }

    private JavaGenerator generator()
    {
        return new JavaGenerator(ir, BUFFER_NAME, READ_ONLY_BUFFER_NAME, false, false, false, outputManager);
    }

    private Class<?> compile(final String className) throws Exception
    {
        final String fqClassName = ir.applicableNamespace() + "." + className;
        final Map<String, CharSequence> sources = outputManager.getSources();
        final Class<?> aClass = CompilerUtil.compileInMemory(fqClassName, sources);
        if (aClass == null)
        {
            System.out.println(sources);
        }

        return aClass;
    }

    private Object getTestMessage2Encoder(final UnsafeBuffer buffer) throws Exception
    {
        final Object encoder = compile("TestMessage2Encoder").getConstructor().newInstance();
        return wrap(encoder, buffer);
    }

    private Object getTestMessage2Decoder(final UnsafeBuffer buffer, final Object encoder) throws Exception
    {
        final Object decoder = compile("TestMessage2Decoder").getConstructor().newInstance();
        return wrap(buffer, decoder, getSbeBlockLength(encoder), getSbeSchemaVersion(encoder));
    }

    private static Object wrap(
        final UnsafeBuffer buffer, final Object decoder, final int blockLength, final int version) throws Exception
    {
        decoder
            .getClass()
            .getMethod("wrap", READ_ONLY_BUFFER_CLASS, int.class, int.class, int.class)
            .invoke(decoder, buffer, 0, blockLength, version);

        return decoder;
    }

    private static Object wrap(final Object flyweight, final UnsafeBuffer buffer) throws Exception
    {
        flyweight
            .getClass()
            .getDeclaredMethod("wrap", BUFFER_CLASS, int.class)
            .invoke(flyweight, buffer, 0);

        return flyweight;
    }
}
