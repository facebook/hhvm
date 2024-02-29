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
import java.util.Arrays;
import java.util.Map;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.is;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNull;
import static uk.co.real_logic.sbe.generation.java.ReflectionUtil.*;

@SuppressWarnings("MethodLength")
class SchemaExtensionTest
{
    private static final Class<?> BUFFER_CLASS = MutableDirectBuffer.class;
    private static final String BUFFER_NAME = BUFFER_CLASS.getName();
    private static final Class<DirectBuffer> READ_ONLY_BUFFER_CLASS = DirectBuffer.class;
    private static final String READ_ONLY_BUFFER_NAME = READ_ONLY_BUFFER_CLASS.getName();

    private final StringWriterOutputManager outputManager = new StringWriterOutputManager();

    private Ir ir;

    @BeforeEach
    void setup() throws Exception
    {
        try (InputStream in = Tests.getLocalResource("extension-schema.xml"))
        {
            final ParserOptions options = ParserOptions.builder().stopOnError(true).build();
            final MessageSchema schema = XmlSchemaParser.parse(in, options);
            final IrGenerator irg = new IrGenerator();
            ir = irg.generate(schema);
        }

        outputManager.clear();
        outputManager.setPackageName(ir.applicableNamespace());

        generator().generate();
    }

    @Test
    void testMessage1() throws Exception
    {
        final UnsafeBuffer buffer = new UnsafeBuffer(new byte[4096]);

        { // Encode
            final Object encoder = wrap(buffer, compile("TestMessage1Encoder").getConstructor().newInstance());

            set(encoder, "tag1", int.class, 100);
            set(encoder, "tag2", int.class, 200);

            final Object compositeEncoder = encoder.getClass().getMethod("tag3").invoke(encoder);
            set(compositeEncoder, "value", int.class, 300);

            final Object enumConstant = getAEnumConstant(encoder, "AEnum", 1);
            set(encoder, "tag4", enumConstant.getClass(), enumConstant);

            final Object setEncoder = encoder.getClass().getMethod("tag5").invoke(encoder);
            set(setEncoder, "firstChoice", boolean.class, false);
            set(setEncoder, "secondChoice", boolean.class, true);

            set(encoder, "tag6", String.class, "This is some variable length data");
        }

        { // Decode version 0
            final Object decoderVersion0 = getMessage1Decoder(buffer, 4, 0);
            assertEquals(100, get(decoderVersion0, "tag1"));
            assertEquals(Integer.MIN_VALUE, get(decoderVersion0, "tag2"));
            assertNull(get(decoderVersion0, "tag3"));
            assertThat(get(decoderVersion0, "tag4").toString(), is("NULL_VAL"));
            assertNull(get(decoderVersion0, "tag5"));
            final StringBuilder tag6Value = new StringBuilder();
            get(decoderVersion0, "tag6", tag6Value);
            assertThat(tag6Value.length(), is(0));

            assertEquals(0, decoderVersion0.getClass().getMethod("tag1SinceVersion").invoke(null));
            assertEquals(1, decoderVersion0.getClass().getMethod("tag2SinceVersion").invoke(null));
            assertEquals(2, decoderVersion0.getClass().getMethod("tag3SinceVersion").invoke(null));
            assertEquals(3, decoderVersion0.getClass().getMethod("tag4SinceVersion").invoke(null));
            assertEquals(4, decoderVersion0.getClass().getMethod("tag5SinceVersion").invoke(null));
            assertEquals(5, decoderVersion0.getClass().getMethod("tag6SinceVersion").invoke(null));
        }

        { // Decode version 1
            final Object decoderVersion1 = getMessage1Decoder(buffer, 8, 1);
            assertEquals(100, get(decoderVersion1, "tag1"));
            assertEquals(200, get(decoderVersion1, "tag2"));
            assertNull(get(decoderVersion1, "tag3"));
            assertThat(get(decoderVersion1, "tag4").toString(), is("NULL_VAL"));
            assertNull(get(decoderVersion1, "tag5"));
            final StringBuilder tag6Value = new StringBuilder();
            get(decoderVersion1, "tag6", tag6Value);
            assertThat(tag6Value.length(), is(0));
        }

        { // Decode version 2
            final Object decoderVersion2 = getMessage1Decoder(buffer, 8, 2);
            assertEquals(100, get(decoderVersion2, "tag1"));
            assertEquals(200, get(decoderVersion2, "tag2"));
            final Object compositeDecoder2 = get(decoderVersion2, "tag3");
            assertNotNull(compositeDecoder2);
            assertEquals(300, get(compositeDecoder2, "value"));
            assertThat(get(decoderVersion2, "tag4").toString(), is("NULL_VAL"));
            assertNull(get(decoderVersion2, "tag5"));
            final StringBuilder tag6Value = new StringBuilder();
            get(decoderVersion2, "tag6", tag6Value);
            assertThat(tag6Value.length(), is(0));
        }

        { // Decode version 3
            final Object decoderVersion3 = getMessage1Decoder(buffer, 12, 3);
            assertEquals(100, get(decoderVersion3, "tag1"));
            assertEquals(200, get(decoderVersion3, "tag2"));
            final Object compositeDecoder3 = get(decoderVersion3, "tag3");
            assertNotNull(compositeDecoder3);
            assertEquals(300, get(compositeDecoder3, "value"));
            final Object enumConstant = getAEnumConstant(decoderVersion3, "AEnum", 1);
            assertEquals(enumConstant, get(decoderVersion3, "tag4"));
            assertNull(get(decoderVersion3, "tag5"));
            final StringBuilder tag6Value = new StringBuilder();
            get(decoderVersion3, "tag6", tag6Value);
            assertThat(tag6Value.length(), is(0));
        }

        { // Decode version 4
            final Object decoderVersion4 = getMessage1Decoder(buffer, 12, 4);
            assertEquals(100, get(decoderVersion4, "tag1"));
            assertEquals(200, get(decoderVersion4, "tag2"));
            final Object compositeDecoder4 = get(decoderVersion4, "tag3");
            assertNotNull(compositeDecoder4);
            assertEquals(300, get(compositeDecoder4, "value"));
            final Object enumConstant = getAEnumConstant(decoderVersion4, "AEnum", 1);
            assertEquals(enumConstant, get(decoderVersion4, "tag4"));
            final Object setDecoder = get(decoderVersion4, "tag5");
            assertNotNull(setDecoder);
            assertEquals(false, get(setDecoder, "firstChoice"));
            assertEquals(true, get(setDecoder, "secondChoice"));
            final StringBuilder tag6Value = new StringBuilder();
            get(decoderVersion4, "tag6", tag6Value);
            assertThat(tag6Value.length(), is(0));
        }

        { // Decode version 5
            final Object decoderVersion5 = getMessage1Decoder(buffer, 14, 5);
            assertEquals(100, get(decoderVersion5, "tag1"));
            assertEquals(200, get(decoderVersion5, "tag2"));
            final Object compositeDecoder4 = get(decoderVersion5, "tag3");
            assertNotNull(compositeDecoder4);
            assertEquals(300, get(compositeDecoder4, "value"));
            final Object enumConstant = getAEnumConstant(decoderVersion5, "AEnum", 1);
            assertEquals(enumConstant, get(decoderVersion5, "tag4"));
            final Object setDecoder = get(decoderVersion5, "tag5");
            assertNotNull(setDecoder);
            assertEquals(false, get(setDecoder, "firstChoice"));
            assertEquals(true, get(setDecoder, "secondChoice"));
            final StringBuilder tag6Value = new StringBuilder();
            get(decoderVersion5, "tag6", tag6Value);
            assertThat(tag6Value.toString(), is("This is some variable length data"));
        }
    }

    @Test
    void testMessage2() throws Exception
    {
        final UnsafeBuffer buffer = new UnsafeBuffer(new byte[4096]);

        { // Encode
            final Object encoder = wrap(buffer, compile("TestMessage2Encoder").getConstructor().newInstance());

            set(encoder, "tag1", int.class, 100);
            set(encoder, "tag2", int.class, 200);

            final Object compositeEncoder = encoder.getClass().getMethod("tag3").invoke(encoder);
            set(compositeEncoder, "value", int.class, 300);

            final Object enumConstant = getAEnumConstant(encoder, "AEnum", 1);
            set(encoder, "tag4", enumConstant.getClass(), enumConstant);

            final Object setEncoder = encoder.getClass().getMethod("tag5").invoke(encoder);
            set(setEncoder, "firstChoice", boolean.class, false);
            set(setEncoder, "secondChoice", boolean.class, true);

            final byte[] data = "  **DATA**  ".getBytes(StandardCharsets.US_ASCII);
            putByteArray(encoder, "putTag6", data, 2, 8);
        }

        { // Decode version 0
            final byte[] data = new byte[12];
            Arrays.fill(data, (byte)' ');
            final UnsafeBuffer dataBuffer = new UnsafeBuffer(new byte[12]);
            dataBuffer.setMemory(0, 12, (byte)' ');

            final Object decoderVersion0 = getMessage2Decoder(buffer, 4, 0);
            assertEquals(100, get(decoderVersion0, "tag1"));
            assertEquals(Integer.MIN_VALUE, get(decoderVersion0, "tag2"));
            assertNull(get(decoderVersion0, "tag3"));
            assertThat(get(decoderVersion0, "tag4").toString(), is("NULL_VAL"));
            assertNull(get(decoderVersion0, "tag5"));
            // tag6 (fixed-size data)
            assertEquals(0, getByteArray(decoderVersion0, "getTag6", data, 1, 8));
            assertEquals("            ", new String(data, StandardCharsets.US_ASCII));
            assertEquals(0, getDirectBuffer(decoderVersion0, "getTag6", dataBuffer, 3, 8));
            assertEquals("            ", dataBuffer.getStringWithoutLengthAscii(0, 12));
            wrapDirectBuffer(decoderVersion0, "wrapTag6", dataBuffer);
            assertEquals("", dataBuffer.getStringWithoutLengthAscii(0, dataBuffer.capacity()));

            assertEquals(0, decoderVersion0.getClass().getMethod("tag1SinceVersion").invoke(null));
            assertEquals(2, decoderVersion0.getClass().getMethod("tag2SinceVersion").invoke(null));
            assertEquals(1, decoderVersion0.getClass().getMethod("tag3SinceVersion").invoke(null));
            assertEquals(4, decoderVersion0.getClass().getMethod("tag4SinceVersion").invoke(null));
            assertEquals(3, decoderVersion0.getClass().getMethod("tag5SinceVersion").invoke(null));
            assertEquals(6, decoderVersion0.getClass().getMethod("tag6SinceVersion").invoke(null));
        }

        { // Decode version 1
            final byte[] data = new byte[12];
            Arrays.fill(data, (byte)' ');
            final UnsafeBuffer dataBuffer = new UnsafeBuffer(new byte[12]);
            dataBuffer.setMemory(0, 12, (byte)' ');

            final Object decoderVersion1 = getMessage2Decoder(buffer, 8, 1);
            assertEquals(100, get(decoderVersion1, "tag1"));
            assertEquals(Integer.MIN_VALUE, get(decoderVersion1, "tag2"));
            final Object compositeDecoder2 = get(decoderVersion1, "tag3");
            assertNotNull(compositeDecoder2);
            assertEquals(300, get(compositeDecoder2, "value"));
            assertThat(get(decoderVersion1, "tag4").toString(), is("NULL_VAL"));
            assertNull(get(decoderVersion1, "tag5"));
            // tag6 (fixed-size data)
            assertEquals(0, getByteArray(decoderVersion1, "getTag6", data, 1, 8));
            assertEquals("            ", new String(data, StandardCharsets.US_ASCII));
            assertEquals(0, getDirectBuffer(decoderVersion1, "getTag6", dataBuffer, 3, 8));
            assertEquals("            ", dataBuffer.getStringWithoutLengthAscii(0, 12));
            wrapDirectBuffer(decoderVersion1, "wrapTag6", dataBuffer);
            assertEquals("", dataBuffer.getStringWithoutLengthAscii(0, dataBuffer.capacity()));
        }

        { // Decode version 2
            final byte[] data = new byte[12];
            Arrays.fill(data, (byte)' ');
            final UnsafeBuffer dataBuffer = new UnsafeBuffer(new byte[12]);
            dataBuffer.setMemory(0, 12, (byte)' ');

            final Object decoderVersion2 = getMessage2Decoder(buffer, 12, 2);
            assertEquals(100, get(decoderVersion2, "tag1"));
            assertEquals(200, get(decoderVersion2, "tag2"));
            final Object compositeDecoder2 = get(decoderVersion2, "tag3");
            assertNotNull(compositeDecoder2);
            assertEquals(300, get(compositeDecoder2, "value"));
            assertThat(get(decoderVersion2, "tag4").toString(), is("NULL_VAL"));
            assertNull(get(decoderVersion2, "tag5"));
            // tag6 (fixed-size data)
            assertEquals(0, getByteArray(decoderVersion2, "getTag6", data, 1, 8));
            assertEquals("            ", new String(data, StandardCharsets.US_ASCII));
            assertEquals(0, getDirectBuffer(decoderVersion2, "getTag6", dataBuffer, 3, 8));
            assertEquals("            ", dataBuffer.getStringWithoutLengthAscii(0, 12));
            wrapDirectBuffer(decoderVersion2, "wrapTag6", dataBuffer);
            assertEquals("", dataBuffer.getStringWithoutLengthAscii(0, dataBuffer.capacity()));
        }

        { // Decode version 3
            final byte[] data = new byte[12];
            Arrays.fill(data, (byte)' ');
            final UnsafeBuffer dataBuffer = new UnsafeBuffer(new byte[12]);
            dataBuffer.setMemory(0, 12, (byte)' ');

            final Object decoderVersion3 = getMessage2Decoder(buffer, 13, 3);
            assertEquals(100, get(decoderVersion3, "tag1"));
            assertEquals(200, get(decoderVersion3, "tag2"));
            final Object compositeDecoder3 = get(decoderVersion3, "tag3");
            assertNotNull(compositeDecoder3);
            assertEquals(300, get(compositeDecoder3, "value"));
            assertThat(get(decoderVersion3, "tag4").toString(), is("NULL_VAL"));
            final Object setDecoder = get(decoderVersion3, "tag5");
            assertNotNull(setDecoder);
            assertEquals(false, get(setDecoder, "firstChoice"));
            assertEquals(true, get(setDecoder, "secondChoice"));
            // tag6 (fixed-size data)
            assertEquals(0, getByteArray(decoderVersion3, "getTag6", data, 1, 8));
            assertEquals("            ", new String(data, StandardCharsets.US_ASCII));
            assertEquals(0, getDirectBuffer(decoderVersion3, "getTag6", dataBuffer, 3, 8));
            assertEquals("            ", dataBuffer.getStringWithoutLengthAscii(0, 12));
            wrapDirectBuffer(decoderVersion3, "wrapTag6", dataBuffer);
            assertEquals("", dataBuffer.getStringWithoutLengthAscii(0, dataBuffer.capacity()));
        }

        { // Decode version 4
            final byte[] data = new byte[12];
            Arrays.fill(data, (byte)' ');
            final UnsafeBuffer dataBuffer = new UnsafeBuffer(new byte[12]);
            dataBuffer.setMemory(0, 12, (byte)' ');

            final Object decoderVersion4 = getMessage2Decoder(buffer, 14, 4);
            assertEquals(100, get(decoderVersion4, "tag1"));
            assertEquals(200, get(decoderVersion4, "tag2"));
            final Object compositeDecoder4 = get(decoderVersion4, "tag3");
            assertNotNull(compositeDecoder4);
            assertEquals(300, get(compositeDecoder4, "value"));
            final Object enumConstant = getAEnumConstant(decoderVersion4, "AEnum", 1);
            assertEquals(enumConstant, get(decoderVersion4, "tag4"));
            final Object setDecoder = get(decoderVersion4, "tag5");
            assertNotNull(setDecoder);
            assertEquals(false, get(setDecoder, "firstChoice"));
            assertEquals(true, get(setDecoder, "secondChoice"));
            // tag6 (fixed-size data)
            assertEquals(0, getByteArray(decoderVersion4, "getTag6", data, 1, 8));
            assertEquals("            ", new String(data, StandardCharsets.US_ASCII));
            assertEquals(0, getDirectBuffer(decoderVersion4, "getTag6", dataBuffer, 3, 8));
            assertEquals("            ", dataBuffer.getStringWithoutLengthAscii(0, 12));
            wrapDirectBuffer(decoderVersion4, "wrapTag6", dataBuffer);
            assertEquals("", dataBuffer.getStringWithoutLengthAscii(0, dataBuffer.capacity()));
        }

        { // Decode version 5
            final byte[] data = new byte[12];
            Arrays.fill(data, (byte)' ');
            final UnsafeBuffer dataBuffer = new UnsafeBuffer(new byte[12]);
            dataBuffer.setMemory(0, 12, (byte)' ');

            final Object decoderVersion5 = getMessage2Decoder(buffer, 14, 5);
            assertEquals(100, get(decoderVersion5, "tag1"));
            assertEquals(200, get(decoderVersion5, "tag2"));
            final Object compositeDecoder4 = get(decoderVersion5, "tag3");
            assertNotNull(compositeDecoder4);
            assertEquals(300, get(compositeDecoder4, "value"));
            final Object enumConstant = getAEnumConstant(decoderVersion5, "AEnum", 1);
            assertEquals(enumConstant, get(decoderVersion5, "tag4"));
            final Object setDecoder = get(decoderVersion5, "tag5");
            assertNotNull(setDecoder);
            assertEquals(false, get(setDecoder, "firstChoice"));
            assertEquals(true, get(setDecoder, "secondChoice"));
            // tag6 (fixed-size data)
            assertEquals(0, getByteArray(decoderVersion5, "getTag6", data, 1, 8));
            assertEquals("            ", new String(data, StandardCharsets.US_ASCII));
            assertEquals(0, getDirectBuffer(decoderVersion5, "getTag6", dataBuffer, 3, 8));
            assertEquals("            ", dataBuffer.getStringWithoutLengthAscii(0, 12));
            wrapDirectBuffer(decoderVersion5, "wrapTag6", dataBuffer);
            assertEquals("", dataBuffer.getStringWithoutLengthAscii(0, dataBuffer.capacity()));
        }

        { // Decode version 6
            final byte[] data = new byte[12];
            Arrays.fill(data, (byte)' ');
            final UnsafeBuffer dataBuffer = new UnsafeBuffer(new byte[12]);
            dataBuffer.setMemory(0, 12, (byte)' ');

            final Object decoderVersion6 = getMessage2Decoder(buffer, 22, 6);
            assertEquals(100, get(decoderVersion6, "tag1"));
            assertEquals(200, get(decoderVersion6, "tag2"));
            final Object compositeDecoder4 = get(decoderVersion6, "tag3");
            assertNotNull(compositeDecoder4);
            assertEquals(300, get(compositeDecoder4, "value"));
            final Object enumConstant = getAEnumConstant(decoderVersion6, "AEnum", 1);
            assertEquals(enumConstant, get(decoderVersion6, "tag4"));
            final Object setDecoder = get(decoderVersion6, "tag5");
            assertNotNull(setDecoder);
            assertEquals(false, get(setDecoder, "firstChoice"));
            assertEquals(true, get(setDecoder, "secondChoice"));
            // tag6 (fixed-size data)
            assertEquals(8, getByteArray(decoderVersion6, "getTag6", data, 1, 8));
            assertEquals(" **DATA**   ", new String(data, StandardCharsets.US_ASCII));
            assertEquals(8, getDirectBuffer(decoderVersion6, "getTag6", dataBuffer, 3, 8));
            assertEquals("   **DATA** ", dataBuffer.getStringWithoutLengthAscii(0, 12));
            wrapDirectBuffer(decoderVersion6, "wrapTag6", dataBuffer);
            assertEquals("**DATA**", dataBuffer.getStringWithoutLengthAscii(0, dataBuffer.capacity()));
        }
    }

    private JavaGenerator generator()
    {
        return new JavaGenerator(ir, BUFFER_NAME, READ_ONLY_BUFFER_NAME, false, false, false, outputManager);
    }

    private Object getMessage1Decoder(final UnsafeBuffer buffer, final int blockLength, final int version)
        throws Exception
    {
        final Object decoder = compile("TestMessage1Decoder").getConstructor().newInstance();
        return wrap(buffer, decoder, blockLength, version);
    }

    private Object getMessage2Decoder(final UnsafeBuffer buffer, final int blockLength, final int version)
        throws Exception
    {
        final Object decoder = compile("TestMessage2Decoder").getConstructor().newInstance();
        return wrap(buffer, decoder, blockLength, version);
    }

    private Object getAEnumConstant(
        final Object flyweight, final String enumClassName, final int constantIndex) throws Exception
    {
        final String fqClassName = ir.applicableNamespace() + "." + enumClassName;
        return flyweight.getClass().getClassLoader().loadClass(fqClassName).getEnumConstants()[constantIndex];
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
        assertNotNull(aClass);

        return aClass;
    }

    private static Object wrap(
        final UnsafeBuffer buffer, final Object decoder, final int blockLength, final int version) throws Exception
    {
        return wrap(buffer, decoder, blockLength, version, READ_ONLY_BUFFER_CLASS);
    }

    private static Object wrap(
        final UnsafeBuffer buffer,
        final Object decoder,
        final int blockLength,
        final int version,
        final Class<?> bufferClass) throws Exception
    {
        decoder
            .getClass()
            .getMethod("wrap", bufferClass, int.class, int.class, int.class)
            .invoke(decoder, buffer, 0, blockLength, version);

        return decoder;
    }

    private static void wrap(final Object flyweight, final UnsafeBuffer buffer, final Class<?> bufferClass)
        throws Exception
    {
        flyweight
            .getClass()
            .getDeclaredMethod("wrap", bufferClass, int.class)
            .invoke(flyweight, buffer, 0);
    }

    private static Object wrap(final UnsafeBuffer buffer, final Object encoder) throws Exception
    {
        wrap(encoder, buffer, BUFFER_CLASS);

        return encoder;
    }
}
