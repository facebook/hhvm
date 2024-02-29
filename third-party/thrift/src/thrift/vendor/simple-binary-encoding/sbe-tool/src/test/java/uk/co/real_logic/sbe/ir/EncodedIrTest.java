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
package uk.co.real_logic.sbe.ir;

import org.junit.jupiter.api.Test;
import uk.co.real_logic.sbe.Tests;
import uk.co.real_logic.sbe.xml.IrGenerator;
import uk.co.real_logic.sbe.xml.MessageSchema;
import uk.co.real_logic.sbe.xml.ParserOptions;

import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.List;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.greaterThan;
import static org.hamcrest.core.Is.is;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.parse;

class EncodedIrTest
{
    private static final int CAPACITY = 1024 * 16;

    @Test
    void shouldEncodeIr() throws Exception
    {
        try (InputStream in = Tests.getLocalResource("basic-schema.xml"))
        {
            final MessageSchema schema = parse(in, ParserOptions.DEFAULT);
            final IrGenerator irg = new IrGenerator();
            final Ir ir = irg.generate(schema);
            final ByteBuffer buffer = ByteBuffer.allocate(CAPACITY);
            try (IrEncoder irEncoder = new IrEncoder(buffer, ir))
            {
                irEncoder.encode();
                assertThat(buffer.position(), greaterThan(0));
            }
        }
    }

    @Test
    void shouldEncodeThenDecodeIr() throws Exception
    {
        try (InputStream in = Tests.getLocalResource("basic-schema.xml"))
        {
            final MessageSchema schema = parse(in, ParserOptions.DEFAULT);
            final IrGenerator irg = new IrGenerator();
            final Ir ir = irg.generate(schema);
            final ByteBuffer buffer = ByteBuffer.allocate(CAPACITY);
            try (IrEncoder irEncoder = new IrEncoder(buffer, ir))
            {
                irEncoder.encode();
            }

            buffer.flip();

            try (IrDecoder decoder = new IrDecoder(buffer))
            {
                decoder.decode();
            }
        }
    }

    @Test
    void shouldHandleRightSizedBuffer() throws Exception
    {
        try (InputStream in = Tests.getLocalResource("basic-schema.xml"))
        {
            final MessageSchema schema = parse(in, ParserOptions.DEFAULT);
            final IrGenerator irg = new IrGenerator();
            final Ir ir = irg.generate(schema);
            final ByteBuffer buffer = ByteBuffer.allocate(CAPACITY);

            try (IrEncoder irEncoder = new IrEncoder(buffer, ir))
            {
                irEncoder.encode();
            }

            buffer.flip();

            final ByteBuffer readBuffer = ByteBuffer.allocate(buffer.remaining());
            readBuffer.put(buffer);
            readBuffer.flip();

            try (IrDecoder irDecoder = new IrDecoder(readBuffer))
            {
                irDecoder.decode();
            }
        }
    }

    @Test
    void shouldDecodeFrame() throws Exception
    {
        final InputStream in = Tests.getLocalResource("code-generation-schema.xml");
        {
            final MessageSchema schema = parse(in, ParserOptions.DEFAULT);
            final IrGenerator irg = new IrGenerator();
            final Ir ir = irg.generate(schema);
            final ByteBuffer buffer = ByteBuffer.allocate(CAPACITY);

            try (IrEncoder irEncoder = new IrEncoder(buffer, ir))
            {
                irEncoder.encode();
            }

            buffer.flip();

            try (IrDecoder irDecoder = new IrDecoder(buffer))
            {
                final Ir decodedIr = irDecoder.decode();

                assertThat(decodedIr.id(), is(ir.id()));
                assertThat(decodedIr.version(), is(ir.version()));
                assertThat(decodedIr.semanticVersion(), is(ir.semanticVersion()));
                assertThat(decodedIr.packageName(), is(ir.packageName()));
                assertThat(decodedIr.namespaceName(), is(ir.namespaceName()));
            }
        }
    }

    @Test
    void shouldDecodeHeader() throws Exception
    {
        try (InputStream in = Tests.getLocalResource("code-generation-schema.xml"))
        {
            final MessageSchema schema = parse(in, ParserOptions.DEFAULT);
            final IrGenerator irg = new IrGenerator();
            final Ir ir = irg.generate(schema);
            final ByteBuffer buffer = ByteBuffer.allocate(CAPACITY);

            try (IrEncoder irEncoder = new IrEncoder(buffer, ir))
            {
                irEncoder.encode();
            }

            buffer.flip();

            try (IrDecoder irDecoder = new IrDecoder(buffer))
            {
                final Ir decodedIr = irDecoder.decode();
                final List<Token> tokens = decodedIr.headerStructure().tokens();

                assertThat(tokens.size(), is(ir.headerStructure().tokens().size()));
                for (int i = 0, size = tokens.size(); i < size; i++)
                {
                    assertEqual(tokens.get(i), ir.headerStructure().tokens().get(i));
                }
            }
        }
    }

    @Test
    void shouldDecodeMessagesAndTypes() throws Exception
    {
        try (InputStream in = Tests.getLocalResource("code-generation-schema.xml"))
        {
            final MessageSchema schema = parse(in, ParserOptions.DEFAULT);
            final IrGenerator irg = new IrGenerator();
            final Ir ir = irg.generate(schema);
            final ByteBuffer buffer = ByteBuffer.allocate(CAPACITY);

            try (IrEncoder irEncoder = new IrEncoder(buffer, ir))
            {
                irEncoder.encode();
            }

            buffer.flip();

            try (IrDecoder irDecoder = new IrDecoder(buffer))
            {
                final Ir decodedIr = irDecoder.decode();

                assertThat(decodedIr.messages().size(), is(ir.messages().size()));
                for (final List<Token> decodedTokenList : decodedIr.messages())
                {
                    final List<Token> tokens = ir.getMessage(decodedTokenList.get(0).id());

                    assertThat(decodedTokenList.size(), is(tokens.size()));
                    for (int i = 0, size = decodedTokenList.size(); i < size; i++)
                    {
                        assertEqual(decodedTokenList.get(i), tokens.get(i));
                    }
                }

                assertThat(decodedIr.types().size(), is(ir.types().size()));
                for (final List<Token> decodedTokenList : decodedIr.types())
                {
                    final Token t = decodedTokenList.get(0);
                    final String name = t.referencedName() != null ? t.referencedName() : t.name();
                    final List<Token> tokens = ir.getType(name);

                    assertThat(name + " token count", decodedTokenList.size(), is(tokens.size()));
                    for (int i = 0, size = decodedTokenList.size(); i < size; i++)
                    {
                        assertEqual(decodedTokenList.get(i), tokens.get(i));
                    }
                }
            }
        }
    }

    private void assertEqual(final Token lhs, final Token rhs)
    {
        assertThat(lhs.name(), is(rhs.name()));
        assertThat(lhs.version(), is(rhs.version()));
        assertThat(lhs.offset(), is(rhs.offset()));
        assertThat((long)lhs.id(), is((long)rhs.id()));
        assertThat(lhs.signal(), is(rhs.signal()));
        assertThat(lhs.encodedLength(), is(rhs.encodedLength()));

        assertThat(lhs.encoding().byteOrder(), is(rhs.encoding().byteOrder()));
        assertThat(lhs.encoding().primitiveType(), is(rhs.encoding().primitiveType()));
        assertThat(lhs.encoding().presence(), is(rhs.encoding().presence()));
        assertThat(lhs.encoding().constValue(), is(rhs.encoding().constValue()));
        assertThat(lhs.encoding().minValue(), is(rhs.encoding().minValue()));
        assertThat(lhs.encoding().maxValue(), is(rhs.encoding().maxValue()));
        assertThat(lhs.encoding().nullValue(), is(rhs.encoding().nullValue()));
        assertThat(lhs.encoding().characterEncoding(), is(rhs.encoding().characterEncoding()));
        assertThat(lhs.encoding().epoch(), is(rhs.encoding().epoch()));
        assertThat(lhs.encoding().timeUnit(), is(rhs.encoding().timeUnit()));
        assertThat(lhs.encoding().semanticType(), is(rhs.encoding().semanticType()));
    }
}
