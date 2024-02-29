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

import composite.elements.EnumOne;
import composite.elements.MessageHeaderEncoder;
import composite.elements.MsgDecoder;
import composite.elements.MsgEncoder;
import org.junit.jupiter.api.Test;
import org.mockito.InOrder;
import org.agrona.BitUtil;
import org.agrona.DirectBuffer;
import org.agrona.concurrent.UnsafeBuffer;
import uk.co.real_logic.sbe.ir.Ir;
import uk.co.real_logic.sbe.ir.IrDecoder;
import uk.co.real_logic.sbe.ir.IrEncoder;
import uk.co.real_logic.sbe.ir.generated.MessageHeaderDecoder;
import uk.co.real_logic.sbe.otf.OtfHeaderDecoder;
import uk.co.real_logic.sbe.otf.OtfMessageDecoder;
import uk.co.real_logic.sbe.otf.TokenListener;
import uk.co.real_logic.sbe.xml.IrGenerator;
import uk.co.real_logic.sbe.xml.MessageSchema;
import uk.co.real_logic.sbe.xml.ParserOptions;
import uk.co.real_logic.sbe.xml.XmlSchemaParser;

import java.io.BufferedInputStream;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.core.Is.is;
import static org.hamcrest.Matchers.not;
import static org.hamcrest.Matchers.containsString;
import static org.mockito.Mockito.*;

class CompositeElementsGenerationTest
{
    private static final MessageHeaderEncoder MESSAGE_HEADER = new MessageHeaderEncoder();
    private static final MsgEncoder MSG_ENCODER = new MsgEncoder();
    private static final int MSG_BUFFER_CAPACITY = 4 * 1024;
    private static final int SCHEMA_BUFFER_CAPACITY = 16 * 1024;

    @Test
    void shouldEncode()
    {
        final ByteBuffer encodedMsgBuffer = ByteBuffer.allocate(MSG_BUFFER_CAPACITY);
        encodeTestMessage(encodedMsgBuffer);

        final DirectBuffer decodeBuffer = new UnsafeBuffer(encodedMsgBuffer);

        int offset = 0;
        assertThat(decodeBuffer.getShort(offset), is((short)22));
        offset += BitUtil.SIZE_OF_SHORT;

        assertThat(decodeBuffer.getShort(offset), is((short)1));
        offset += BitUtil.SIZE_OF_SHORT;

        assertThat(decodeBuffer.getShort(offset), is((short)3));
        offset += BitUtil.SIZE_OF_SHORT;

        assertThat(decodeBuffer.getShort(offset), is((short)0));
        offset += BitUtil.SIZE_OF_SHORT;

        assertThat(decodeBuffer.getByte(offset), is((byte)10));
        offset += BitUtil.SIZE_OF_BYTE;

        assertThat(decodeBuffer.getByte(offset), is((byte)42));
        offset += BitUtil.SIZE_OF_BYTE;

        assertThat(decodeBuffer.getInt(offset), is(0x00_01_00_00));
        offset += BitUtil.SIZE_OF_INT;

        assertThat(decodeBuffer.getLong(offset), is(101L));
        offset += BitUtil.SIZE_OF_LONG;

        assertThat(decodeBuffer.getLong(offset), is(202L));
    }

    @Test
    void shouldDecode()
    {
        final ByteBuffer encodedMsgBuffer = ByteBuffer.allocate(MSG_BUFFER_CAPACITY);
        encodeTestMessage(encodedMsgBuffer);

        final DirectBuffer decodeBuffer = new UnsafeBuffer(encodedMsgBuffer);

        final MessageHeaderDecoder hdrDecoder = new MessageHeaderDecoder();
        final MsgDecoder msgDecoder = new MsgDecoder();

        hdrDecoder.wrap(decodeBuffer, 0);
        msgDecoder.wrap(
            decodeBuffer, hdrDecoder.encodedLength(), MSG_ENCODER.sbeBlockLength(), MSG_ENCODER.sbeSchemaVersion());

        assertThat(hdrDecoder.blockLength(), is(22));
        assertThat(hdrDecoder.templateId(), is(1));
        assertThat(hdrDecoder.schemaId(), is(3));
        assertThat(hdrDecoder.version(), is(0));

        assertThat(msgDecoder.structure().enumOne(), is(EnumOne.Value10));
        assertThat(msgDecoder.structure().zeroth(), is((short)42));
        assertThat(msgDecoder.structure().setOne().bit0(), is(false));
        assertThat(msgDecoder.structure().setOne().bit16(), is(true));
        assertThat(msgDecoder.structure().setOne().bit26(), is(false));
        assertThat(msgDecoder.structure().inner().first(), is(101L));
        assertThat(msgDecoder.structure().inner().second(), is(202L));

        assertThat(msgDecoder.encodedLength(), is(22));
    }

    @Test
    void shouldDisplay()
    {
        final ByteBuffer encodedMsgBuffer = ByteBuffer.allocate(MSG_BUFFER_CAPACITY);
        encodeTestMessage(encodedMsgBuffer);

        final String compositeString = MSG_ENCODER.structure().toString();
        assertThat(compositeString, containsString("enumOne="));
        assertThat(compositeString, not(containsString("enumOne=|")));
        assertThat(compositeString, containsString("setOne="));
        assertThat(compositeString, not(containsString("setOne=|")));
    }

    @Test
    void shouldOtfDecode() throws Exception
    {
        final ByteBuffer encodedSchemaBuffer = ByteBuffer.allocate(SCHEMA_BUFFER_CAPACITY);
        encodeSchema(encodedSchemaBuffer);

        final ByteBuffer encodedMsgBuffer = ByteBuffer.allocate(MSG_BUFFER_CAPACITY);
        encodeTestMessage(encodedMsgBuffer);

        encodedSchemaBuffer.flip();
        final Ir ir = decodeIr(encodedSchemaBuffer);

        final DirectBuffer decodeBuffer = new UnsafeBuffer(encodedMsgBuffer);
        final OtfHeaderDecoder otfHeaderDecoder = new OtfHeaderDecoder(ir.headerStructure());

        assertThat(otfHeaderDecoder.getBlockLength(decodeBuffer, 0), is(22));
        assertThat(otfHeaderDecoder.getSchemaId(decodeBuffer, 0), is(3));
        assertThat(otfHeaderDecoder.getTemplateId(decodeBuffer, 0), is(1));
        assertThat(otfHeaderDecoder.getSchemaVersion(decodeBuffer, 0), is(0));

        final TokenListener mockTokenListener = mock(TokenListener.class);

        OtfMessageDecoder.decode(
            decodeBuffer,
            otfHeaderDecoder.encodedLength(),
            MSG_ENCODER.sbeSchemaVersion(),
            MSG_ENCODER.sbeBlockLength(),
            ir.getMessage(MSG_ENCODER.sbeTemplateId()),
            mockTokenListener);

        final InOrder inOrder = inOrder(mockTokenListener);
        inOrder.verify(mockTokenListener).onBeginComposite(any(), any(), eq(2), eq(17));
        inOrder.verify(mockTokenListener).onEnum(any(), eq(decodeBuffer), eq(8), any(), eq(3), eq(6), eq(0));
        inOrder.verify(mockTokenListener).onEncoding(any(), eq(decodeBuffer), eq(9), any(), eq(0));
        inOrder.verify(mockTokenListener).onBitSet(any(), eq(decodeBuffer), eq(10), any(), eq(8), eq(12), eq(0));
        inOrder.verify(mockTokenListener).onBeginComposite(any(), any(), eq(13), eq(16));
        inOrder.verify(mockTokenListener).onEncoding(any(), eq(decodeBuffer), eq(14), any(), eq(0));
        inOrder.verify(mockTokenListener).onEncoding(any(), eq(decodeBuffer), eq(22), any(), eq(0));
        inOrder.verify(mockTokenListener).onEndComposite(any(), any(), eq(13), eq(16));
        inOrder.verify(mockTokenListener).onEndComposite(any(), any(), eq(2), eq(17));
    }

    private static void encodeTestMessage(final ByteBuffer buffer)
    {
        final UnsafeBuffer directBuffer = new UnsafeBuffer(buffer);

        int bufferOffset = 0;
        MESSAGE_HEADER
            .wrap(directBuffer, bufferOffset)
            .blockLength(MSG_ENCODER.sbeBlockLength())
            .templateId(MSG_ENCODER.sbeTemplateId())
            .schemaId(MSG_ENCODER.sbeSchemaId())
            .version(MSG_ENCODER.sbeSchemaVersion());

        bufferOffset += MESSAGE_HEADER.encodedLength();

        MSG_ENCODER.wrap(directBuffer, bufferOffset).structure()
            .enumOne(EnumOne.Value10)
            .zeroth((byte)42);

        MSG_ENCODER.structure()
            .setOne().clear().bit0(false).bit16(true).bit26(false);

        MSG_ENCODER.structure().inner()
            .first(101)
            .second(202);
    }

    private static void encodeSchema(final ByteBuffer buffer) throws Exception
    {
        final Path path = Paths.get("src/test/resources/composite-elements-schema.xml");
        try (InputStream in = new BufferedInputStream(Files.newInputStream(path)))
        {
            final MessageSchema schema = XmlSchemaParser.parse(in, ParserOptions.DEFAULT);
            final Ir ir = new IrGenerator().generate(schema);

            try (IrEncoder irEncoder = new IrEncoder(buffer, ir))
            {
                irEncoder.encode();
            }
        }
    }

    private static Ir decodeIr(final ByteBuffer buffer)
    {
        try (IrDecoder irDecoder = new IrDecoder(buffer))
        {
            return irDecoder.decode();
        }
    }
}
