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
import java.util.List;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.core.Is.is;
import static uk.co.real_logic.sbe.Tests.getLocalResource;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.parse;

class CompositeElementsIrTest
{
    @Test
    void shouldGenerateIrForCompositeElementsSchema() throws Exception
    {
        try (InputStream in = getLocalResource("composite-elements-schema.xml"))
        {
            final MessageSchema schema = parse(in, ParserOptions.DEFAULT);
            final IrGenerator irg = new IrGenerator();
            final Ir ir = irg.generate(schema);
            final List<Token> tokens = ir.getMessage(1);

            final Token fieldToken = tokens.get(1);
            final Token outerCompositeToken = tokens.get(2);
            final Token enumToken = tokens.get(3);
            final Token zerothToken = tokens.get(7);
            final Token setToken = tokens.get(8);
            final Token innerCompositeToken = tokens.get(13);
            final Token firstToken = tokens.get(14);
            final Token secondToken = tokens.get(15);
            final Token endOuterCompositeToken = tokens.get(17);

            assertThat(fieldToken.signal(), is(Signal.BEGIN_FIELD));
            assertThat(fieldToken.name(), is("structure"));

            assertThat(outerCompositeToken.signal(), is(Signal.BEGIN_COMPOSITE));
            assertThat(outerCompositeToken.name(), is("outer"));
            assertThat(outerCompositeToken.componentTokenCount(), is(16));

            assertThat(enumToken.signal(), is(Signal.BEGIN_ENUM));
            assertThat(enumToken.name(), is("enumOne"));
            assertThat(enumToken.encodedLength(), is(1));
            assertThat(enumToken.encoding().primitiveType(), is(PrimitiveType.UINT8));
            assertThat(enumToken.offset(), is(0));
            assertThat(enumToken.componentTokenCount(), is(4));

            assertThat(zerothToken.signal(), is(Signal.ENCODING));
            assertThat(zerothToken.offset(), is(1));
            assertThat(zerothToken.encoding().primitiveType(), is(PrimitiveType.UINT8));

            assertThat(setToken.signal(), is(Signal.BEGIN_SET));
            assertThat(setToken.name(), is("setOne"));
            assertThat(setToken.encodedLength(), is(4));
            assertThat(setToken.encoding().primitiveType(), is(PrimitiveType.UINT32));
            assertThat(setToken.offset(), is(2));
            assertThat(setToken.componentTokenCount(), is(5));

            assertThat(innerCompositeToken.signal(), is(Signal.BEGIN_COMPOSITE));
            assertThat(innerCompositeToken.name(), is("inner"));
            assertThat(innerCompositeToken.offset(), is(6));
            assertThat(innerCompositeToken.componentTokenCount(), is(4));

            assertThat(firstToken.signal(), is(Signal.ENCODING));
            assertThat(firstToken.name(), is("first"));
            assertThat(firstToken.offset(), is(0));
            assertThat(firstToken.encoding().primitiveType(), is(PrimitiveType.INT64));

            assertThat(secondToken.signal(), is(Signal.ENCODING));
            assertThat(secondToken.name(), is("second"));
            assertThat(secondToken.offset(), is(8));
            assertThat(secondToken.encoding().primitiveType(), is(PrimitiveType.INT64));

            assertThat(endOuterCompositeToken.signal(), is(Signal.END_COMPOSITE));
            assertThat(endOuterCompositeToken.name(), is("outer"));
        }
    }

    @Test
    void shouldGenerateIrForCompositeElementsWithOffsetsSchemaRc4() throws Exception
    {
        try (InputStream in = getLocalResource("composite-elements-schema-rc4.xml"))
        {
            final MessageSchema schema = parse(in, ParserOptions.DEFAULT);
            final IrGenerator irg = new IrGenerator();
            final Ir ir = irg.generate(schema);
            final List<Token> tokens = ir.getMessage(2);

            final Token outerCompositeToken = tokens.get(2);
            final Token enumToken = tokens.get(3);
            final Token zerothToken = tokens.get(7);
            final Token setToken = tokens.get(8);
            final Token innerCompositeToken = tokens.get(13);
            final Token firstToken = tokens.get(14);
            final Token secondToken = tokens.get(15);
            final Token endOuterCompositeToken = tokens.get(17);

            assertThat(outerCompositeToken.signal(), is(Signal.BEGIN_COMPOSITE));
            assertThat(outerCompositeToken.name(), is("outerWithOffsets"));
            assertThat(outerCompositeToken.encodedLength(), is(32));

            assertThat(enumToken.signal(), is(Signal.BEGIN_ENUM));
            assertThat(enumToken.encodedLength(), is(1));
            assertThat(enumToken.encoding().primitiveType(), is(PrimitiveType.UINT8));
            assertThat(enumToken.offset(), is(2));

            assertThat(zerothToken.signal(), is(Signal.ENCODING));
            assertThat(zerothToken.offset(), is(3));
            assertThat(zerothToken.encoding().primitiveType(), is(PrimitiveType.UINT8));

            assertThat(setToken.signal(), is(Signal.BEGIN_SET));
            assertThat(setToken.name(), is("setOne"));
            assertThat(setToken.encodedLength(), is(4));
            assertThat(setToken.encoding().primitiveType(), is(PrimitiveType.UINT32));
            assertThat(setToken.offset(), is(8));

            assertThat(innerCompositeToken.signal(), is(Signal.BEGIN_COMPOSITE));
            assertThat(innerCompositeToken.name(), is("inner"));
            assertThat(innerCompositeToken.offset(), is(16));

            assertThat(firstToken.signal(), is(Signal.ENCODING));
            assertThat(firstToken.name(), is("first"));
            assertThat(firstToken.offset(), is(0));
            assertThat(firstToken.encoding().primitiveType(), is(PrimitiveType.INT64));

            assertThat(secondToken.signal(), is(Signal.ENCODING));
            assertThat(secondToken.name(), is("second"));
            assertThat(secondToken.offset(), is(8));
            assertThat(secondToken.encoding().primitiveType(), is(PrimitiveType.INT64));

            assertThat(endOuterCompositeToken.signal(), is(Signal.END_COMPOSITE));
            assertThat(endOuterCompositeToken.name(), is("outerWithOffsets"));
        }
    }

    @Test
    void shouldGenerateIrForCompositeWithRefSchema() throws Exception
    {
        try (InputStream in = getLocalResource("composite-elements-schema-rc4.xml"))
        {
            final MessageSchema schema = parse(in, ParserOptions.DEFAULT);
            final IrGenerator irg = new IrGenerator();
            final Ir ir = irg.generate(schema);
            final List<Token> tokens = ir.getMessage(3);

            final Token beginCompositeToken = tokens.get(2);
            final Token mantissaToken = tokens.get(3);
            final Token exponentToken = tokens.get(4);
            final Token enumToken = tokens.get(5);
            final Token endCompositeToken = tokens.get(9);

            assertThat(beginCompositeToken.signal(), is(Signal.BEGIN_COMPOSITE));
            assertThat(beginCompositeToken.name(), is("futuresPrice"));
            assertThat(beginCompositeToken.encodedLength(), is(11));

            assertThat(mantissaToken.signal(), is(Signal.ENCODING));
            assertThat(mantissaToken.name(), is("mantissa"));
            assertThat(mantissaToken.offset(), is(0));
            assertThat(mantissaToken.encoding().primitiveType(), is(PrimitiveType.INT64));

            assertThat(exponentToken.signal(), is(Signal.ENCODING));
            assertThat(exponentToken.name(), is("exponent"));
            assertThat(exponentToken.offset(), is(8));
            assertThat(exponentToken.encoding().primitiveType(), is(PrimitiveType.INT8));

            assertThat(enumToken.signal(), is(Signal.BEGIN_ENUM));
            assertThat(enumToken.encodedLength(), is(1));
            assertThat(enumToken.encoding().primitiveType(), is(PrimitiveType.UINT8));
            assertThat(enumToken.offset(), is(10));

            assertThat(endCompositeToken.signal(), is(Signal.END_COMPOSITE));
            assertThat(endCompositeToken.name(), is("futuresPrice"));
        }
    }
}
