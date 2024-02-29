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
import org.agrona.generation.CompilerUtil;
import org.agrona.generation.StringWriterOutputManager;
import org.junit.jupiter.api.Test;
import uk.co.real_logic.sbe.SbeTool;
import uk.co.real_logic.sbe.Tests;
import uk.co.real_logic.sbe.ir.Ir;
import uk.co.real_logic.sbe.ir.IrDecoder;
import uk.co.real_logic.sbe.ir.IrEncoder;
import uk.co.real_logic.sbe.xml.IrGenerator;
import uk.co.real_logic.sbe.xml.MessageSchema;
import uk.co.real_logic.sbe.xml.ParserOptions;

import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.Map;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.parse;

class GenerateIlinkBinaryTest
{
    private static final Class<?> BUFFER_CLASS = MutableDirectBuffer.class;
    private static final String BUFFER_NAME = BUFFER_CLASS.getName();
    private static final Class<DirectBuffer> READ_ONLY_BUFFER_CLASS = DirectBuffer.class;
    private static final String READ_ONLY_BUFFER_NAME = READ_ONLY_BUFFER_CLASS.getName();

    private final StringWriterOutputManager outputManager = new StringWriterOutputManager();

    @Test
    void shouldGenerateValidJava() throws Exception
    {
        System.setProperty(SbeTool.KEYWORD_APPEND_TOKEN, "_");

        try (InputStream in = Tests.getLocalResource("ilinkbinary.xml"))
        {
            final ParserOptions options = ParserOptions.builder().stopOnError(true).build();
            final MessageSchema schema = parse(in, options);
            final IrGenerator irg = new IrGenerator();
            final Ir ir = irg.generate(schema);
            final JavaGenerator generator = new JavaGenerator(
                ir, BUFFER_NAME, READ_ONLY_BUFFER_NAME, false, false, false, outputManager);

            outputManager.setPackageName(ir.applicableNamespace());
            generator.generateMessageHeaderStub();
            generator.generateTypeStubs();
            generator.generate();

            final Map<String, CharSequence> sources = outputManager.getSources();

            {
                final String className = "NewOrderSingle514Encoder";
                final String fqClassName = ir.applicableNamespace() + "." + className;
                final Class<?> aClass = CompilerUtil.compileInMemory(fqClassName, sources);
                assertNotNull(aClass);
            }

            {
                final String className = "NewOrderSingle514Decoder";
                final String fqClassName = ir.applicableNamespace() + "." + className;
                final Class<?> aClass = CompilerUtil.compileInMemory(fqClassName, sources);
                assertNotNull(aClass);
            }
        }
    }

    @Test
    void shouldGenerateAndEncodeIr() throws Exception
    {
        System.setProperty(SbeTool.KEYWORD_APPEND_TOKEN, "_");

        try (InputStream in = Tests.getLocalResource("ilinkbinary.xml"))
        {
            final ParserOptions options = ParserOptions.builder().stopOnError(true).build();
            final MessageSchema schema = parse(in, options);
            final IrGenerator irg = new IrGenerator();
            final Ir ir = irg.generate(schema);
            final ByteBuffer buffer = ByteBuffer.allocate(1024 * 1024);

            try (IrEncoder irEncoder = new IrEncoder(buffer, ir))
            {
                irEncoder.encode();
            }

            buffer.flip();

            try (IrDecoder irDecoder = new IrDecoder(buffer))
            {
                final Ir decodedIr = irDecoder.decode();

                assertEquals(ir.id(), decodedIr.id());
                assertEquals(ir.version(), decodedIr.version());
                assertEquals(ir.byteOrder(), decodedIr.byteOrder());
                assertEquals(ir.applicableNamespace(), decodedIr.applicableNamespace());
                assertEquals(ir.packageName(), decodedIr.packageName());
                assertEquals(ir.types().size(), decodedIr.types().size());
                assertEquals(ir.messages().size(), decodedIr.messages().size());
            }
        }
    }
}
