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
import uk.co.real_logic.sbe.PrimitiveType;
import uk.co.real_logic.sbe.xml.IrGenerator;
import uk.co.real_logic.sbe.xml.MessageSchema;
import uk.co.real_logic.sbe.xml.ParserOptions;

import java.io.InputStream;
import java.nio.ByteOrder;
import java.util.List;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.core.Is.is;
import static uk.co.real_logic.sbe.Tests.getLocalResource;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.parse;

class BasicXmlIrGenerationTest
{
    @Test
    void shouldGenerateIrForMessageHeader() throws Exception
    {
        try (InputStream in = getLocalResource("basic-schema.xml"))
        {
            final MessageSchema schema = parse(in, ParserOptions.DEFAULT);
            final IrGenerator irg = new IrGenerator();

            final Ir ir = irg.generate(schema);
            final List<Token> tokens = ir.headerStructure().tokens();

            assertThat(tokens.size(), is(6));

            /* assert all elements of node 0 */
            assertThat(tokens.get(0).signal(), is(Signal.BEGIN_COMPOSITE));
            assertThat(tokens.get(0).name(), is("messageHeader"));
            assertThat(tokens.get(0).id(), is(Token.INVALID_ID));
            assertThat(tokens.get(0).encodedLength(), is(8));
            assertThat(tokens.get(0).offset(), is(0));

            /* assert all elements of node 1 */
            assertThat(tokens.get(1).signal(), is(Signal.ENCODING));
            assertThat(tokens.get(1).name(), is("blockLength"));
            assertThat(tokens.get(1).encoding().primitiveType(), is(PrimitiveType.UINT16));
            assertThat(tokens.get(1).id(), is(Token.INVALID_ID));
            assertThat(tokens.get(1).encodedLength(), is(2));
            assertThat(tokens.get(1).offset(), is(0));
            assertThat(tokens.get(1).encoding().byteOrder(), is(ByteOrder.LITTLE_ENDIAN));

            /* assert all elements of node 2 */
            assertThat(tokens.get(2).signal(), is(Signal.ENCODING));
            assertThat(tokens.get(2).name(), is("templateId"));
            assertThat(tokens.get(2).encoding().primitiveType(), is(PrimitiveType.UINT16));
            assertThat(tokens.get(2).id(), is(Token.INVALID_ID));
            assertThat(tokens.get(2).encodedLength(), is(2));
            assertThat(tokens.get(2).offset(), is(2));
            assertThat(tokens.get(2).encoding().byteOrder(), is(ByteOrder.LITTLE_ENDIAN));

            /* assert all elements of node 3 */
            assertThat(tokens.get(3).signal(), is(Signal.ENCODING));
            assertThat(tokens.get(3).name(), is("schemaId"));
            assertThat(tokens.get(3).encoding().primitiveType(), is(PrimitiveType.UINT16));
            assertThat(tokens.get(3).id(), is(Token.INVALID_ID));
            assertThat(tokens.get(3).encodedLength(), is(2));
            assertThat(tokens.get(3).offset(), is(4));
            assertThat(tokens.get(3).encoding().byteOrder(), is(ByteOrder.LITTLE_ENDIAN));

            /* assert all elements of node 4 */
            assertThat(tokens.get(4).signal(), is(Signal.ENCODING));
            assertThat(tokens.get(4).name(), is("version"));
            assertThat(tokens.get(4).encoding().primitiveType(), is(PrimitiveType.UINT16));
            assertThat(tokens.get(4).id(), is(Token.INVALID_ID));
            assertThat(tokens.get(4).encodedLength(), is(2));
            assertThat(tokens.get(4).offset(), is(6));
            assertThat(tokens.get(4).encoding().byteOrder(), is(ByteOrder.LITTLE_ENDIAN));

            /* assert all elements of node 6 */
            assertThat(tokens.get(5).signal(), is(Signal.END_COMPOSITE));
            assertThat(tokens.get(5).name(), is("messageHeader"));
            assertThat(tokens.get(5).id(), is(Token.INVALID_ID));
            assertThat(tokens.get(5).encodedLength(), is(8));
            assertThat(tokens.get(5).offset(), is(0));
        }
    }

    @Test
    void shouldGenerateIrForBasicMessage() throws Exception
    {
        try (InputStream in = getLocalResource("basic-schema.xml"))
        {
            final MessageSchema schema = parse(in, ParserOptions.DEFAULT);
            final IrGenerator irg = new IrGenerator();
            final Ir ir = irg.generate(schema);

            final List<Token> tokens = ir.getMessage(50001);

            assertThat(tokens.size(), is(5));

            /* assert all elements of node 0 */
            assertThat(tokens.get(0).signal(), is(Signal.BEGIN_MESSAGE));
            assertThat(tokens.get(0).name(), is("TestMessage50001"));
            assertThat(tokens.get(0).id(), is(50001));
            assertThat(tokens.get(0).encodedLength(), is(16));
            assertThat(tokens.get(0).offset(), is(0));

            /* assert all elements of node 1 */
            assertThat(tokens.get(1).signal(), is(Signal.BEGIN_FIELD));
            assertThat(tokens.get(1).name(), is("Tag40001"));
            assertThat(tokens.get(1).id(), is(40001));
            assertThat(tokens.get(1).encodedLength(), is(4));
            assertThat(tokens.get(1).offset(), is(0));

            /* assert all elements of node 2 */
            assertThat(tokens.get(2).signal(), is(Signal.ENCODING));
            assertThat(tokens.get(2).name(), is("uint32"));
            assertThat(tokens.get(2).encoding().primitiveType(), is(PrimitiveType.UINT32));
            assertThat(tokens.get(2).id(), is(Token.INVALID_ID));
            assertThat(tokens.get(2).encodedLength(), is(4));
            assertThat(tokens.get(2).offset(), is(0));
            assertThat(tokens.get(2).encoding().byteOrder(), is(ByteOrder.LITTLE_ENDIAN));

            /* assert all elements of node 3 */
            assertThat(tokens.get(3).signal(), is(Signal.END_FIELD));
            assertThat(tokens.get(3).name(), is("Tag40001"));
            assertThat(tokens.get(3).id(), is(40001));
            assertThat(tokens.get(3).encodedLength(), is(4));
            assertThat(tokens.get(3).offset(), is(0));

            /* assert all elements of node 4 */
            assertThat(tokens.get(4).signal(), is(Signal.END_MESSAGE));
            assertThat(tokens.get(4).name(), is("TestMessage50001"));
            assertThat(tokens.get(4).id(), is(50001));
            assertThat(tokens.get(4).encodedLength(), is(16));
            assertThat(tokens.get(4).offset(), is(0));
        }
    }

    @Test
    void shouldGenerateIrForMessageWithVariableLengthField() throws Exception
    {
        try (InputStream in = getLocalResource("basic-variable-length-schema.xml"))
        {
            final MessageSchema schema = parse(in, ParserOptions.DEFAULT);
            final IrGenerator irg = new IrGenerator();
            final Ir ir = irg.generate(schema);

            final List<Token> tokens = ir.getMessage(1);

            assertThat(tokens.size(), is(8));

            assertThat(tokens.get(0).signal(), is(Signal.BEGIN_MESSAGE));
            assertThat(tokens.get(0).name(), is("TestMessage1"));
            assertThat(tokens.get(0).id(), is(1));
            assertThat(tokens.get(0).encodedLength(), is(0));
            assertThat(tokens.get(0).offset(), is(0));

            assertThat(tokens.get(1).signal(), is(Signal.BEGIN_VAR_DATA));
            assertThat(tokens.get(1).name(), is("encryptedNewPassword"));
            assertThat(tokens.get(1).id(), is(1404));
            assertThat(tokens.get(1).encodedLength(), is(0));
            assertThat(tokens.get(1).offset(), is(0));

            assertThat(tokens.get(2).signal(), is(Signal.BEGIN_COMPOSITE));
            assertThat(tokens.get(2).name(), is("varDataEncoding"));
            assertThat(tokens.get(2).encodedLength(), is(-1));
            assertThat(tokens.get(2).offset(), is(0));

            assertThat(tokens.get(3).signal(), is(Signal.ENCODING));
            assertThat(tokens.get(3).name(), is("length"));
            assertThat(tokens.get(3).encoding().primitiveType(), is(PrimitiveType.UINT8));
            assertThat(tokens.get(3).id(), is(Token.INVALID_ID));
            assertThat(tokens.get(3).encodedLength(), is(1));
            assertThat(tokens.get(3).offset(), is(0));
            assertThat(tokens.get(3).encoding().byteOrder(), is(ByteOrder.LITTLE_ENDIAN));

            assertThat(tokens.get(4).signal(), is(Signal.ENCODING));
            assertThat(tokens.get(4).name(), is("varData"));
            assertThat(tokens.get(4).encoding().primitiveType(), is(PrimitiveType.CHAR));
            assertThat(tokens.get(4).id(), is(Token.INVALID_ID));
            assertThat(tokens.get(4).encodedLength(), is(-1));
            assertThat(tokens.get(4).offset(), is(1));
            assertThat(tokens.get(4).encoding().byteOrder(), is(ByteOrder.LITTLE_ENDIAN));

            assertThat(tokens.get(5).signal(), is(Signal.END_COMPOSITE));
            assertThat(tokens.get(5).name(), is("varDataEncoding"));
            assertThat(tokens.get(5).encodedLength(), is(-1));
            assertThat(tokens.get(5).offset(), is(0));

            assertThat(tokens.get(6).signal(), is(Signal.END_VAR_DATA));
            assertThat(tokens.get(6).name(), is("encryptedNewPassword"));
            assertThat(tokens.get(6).id(), is(1404));
            assertThat(tokens.get(6).encodedLength(), is(0));
            assertThat(tokens.get(6).offset(), is(0));

            assertThat(tokens.get(7).signal(), is(Signal.END_MESSAGE));
            assertThat(tokens.get(7).name(), is("TestMessage1"));
            assertThat(tokens.get(7).id(), is(1));
            assertThat(tokens.get(7).encodedLength(), is(0));
            assertThat(tokens.get(7).offset(), is(0));
        }
    }

    @Test
    void shouldGenerateIrForMessageWithRepeatingGroupWithEmbeddedDimensions() throws Exception
    {
        try (InputStream in = getLocalResource("basic-group-schema.xml"))
        {
            final MessageSchema schema = parse(in, ParserOptions.DEFAULT);
            final IrGenerator irg = new IrGenerator();
            final Ir ir = irg.generate(schema);

            /* 0=msg, 1=field, 2=enc, 3=fieldend, 4=group, 5=comp, 6=enc, 7=enc, 8=compend, ... */
            final int groupIdx = 4;
            final int dimensionsCompIdx = 5;
            final int dimensionsBlEncIdx = 6;
            final int dimensionsNigEncIdx = 7;

            final List<Token> tokens = ir.getMessage(1);

            /* assert on the group token */
            assertThat(tokens.get(groupIdx).signal(), is(Signal.BEGIN_GROUP));
            assertThat(tokens.get(groupIdx).name(), is("Entries"));
            assertThat(tokens.get(groupIdx).id(), is(2));

            /* assert on the comp token for dimensions */
            assertThat(tokens.get(dimensionsCompIdx).signal(), is(Signal.BEGIN_COMPOSITE));
            assertThat(tokens.get(dimensionsCompIdx).name(), is("groupSizeEncoding"));

            /* assert on the enc token for dimensions blockLength */
            assertThat(tokens.get(dimensionsBlEncIdx).signal(), is(Signal.ENCODING));
            assertThat(tokens.get(dimensionsBlEncIdx).name(), is("blockLength"));

            /* assert on the enc token for dimensions numInGroup */
            assertThat(tokens.get(dimensionsNigEncIdx).signal(), is(Signal.ENCODING));
            assertThat(tokens.get(dimensionsNigEncIdx).name(), is("numInGroup"));
        }
    }

    @Test
    void shouldGenerateIrForMessageWithRepeatingGroupWithEmbeddedDimensionsDefaultDimensionType() throws Exception
    {
        try (InputStream in = getLocalResource("embedded-length-and-count-schema.xml"))
        {
            final MessageSchema schema = parse(in, ParserOptions.DEFAULT);
            final IrGenerator irg = new IrGenerator();
            final Ir ir = irg.generate(schema);

            /* 0=msg, 1=field, 2=enc, 3=fieldend, 4=group, 5=comp, 6=enc, 7=enc, 8=compend, 9=field, ... */
            final int groupIdx = 4;
            final int dimensionsCompIdx = 5;
            final int fieldInGroupIdx = 9;

            final List<Token> tokens = ir.getMessage(1);

            assertThat(tokens.get(groupIdx).signal(), is(Signal.BEGIN_GROUP));
            assertThat(tokens.get(groupIdx).name(), is("ListOrdGrp"));
            assertThat(tokens.get(groupIdx).id(), is(73));

            assertThat(tokens.get(dimensionsCompIdx).signal(), is(Signal.BEGIN_COMPOSITE));
            assertThat(tokens.get(dimensionsCompIdx).name(), is("groupSizeEncoding"));

            assertThat(tokens.get(fieldInGroupIdx).signal(), is(Signal.BEGIN_FIELD));
            assertThat(tokens.get(fieldInGroupIdx).name(), is("ClOrdID"));
        }
    }

    @Test
    void shouldGenerateIrForMessageWithVariableLengthFieldWithEmbeddedLength() throws Exception
    {
        try (InputStream in = getLocalResource("embedded-length-and-count-schema.xml"))
        {
            final MessageSchema schema = parse(in, ParserOptions.DEFAULT);
            final IrGenerator irg = new IrGenerator();
            final Ir ir = irg.generate(schema);

            /* 0=msg, 1=field, 2=enc, 3=fieldend, 4=field, 5=comp, 6=enc, 7=enc, 8=compend, 9=fieldend */
            final int lengthFieldIdx = 4;
            final int lengthEncIdx = 6;
            final int dataEncIdx = 7;

            final List<Token> tokens = ir.getMessage(2);

            /* assert the varDataEncoding field node is formed correctly */
            assertThat(tokens.get(lengthFieldIdx).signal(), is(Signal.BEGIN_VAR_DATA));
            assertThat(tokens.get(lengthFieldIdx).name(), is("EncryptedPassword"));
            assertThat(tokens.get(lengthFieldIdx).id(), is(1402));

            /* assert the length node has correct values */
            assertThat(tokens.get(lengthEncIdx).signal(), is(Signal.ENCODING));
            assertThat(tokens.get(lengthEncIdx).encoding().primitiveType(), is(PrimitiveType.UINT8));

            /* assert the group node has the right IrId and xRefIrId, etc. */
            assertThat(tokens.get(dataEncIdx).signal(), is(Signal.ENCODING));
            assertThat(tokens.get(dataEncIdx).encoding().primitiveType(), is(PrimitiveType.CHAR));
        }
    }
}
