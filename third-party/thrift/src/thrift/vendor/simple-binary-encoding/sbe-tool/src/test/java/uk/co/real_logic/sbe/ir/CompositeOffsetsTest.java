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
import uk.co.real_logic.sbe.xml.IrGenerator;
import uk.co.real_logic.sbe.xml.MessageSchema;
import uk.co.real_logic.sbe.xml.ParserOptions;

import java.io.InputStream;
import java.util.List;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.is;
import static uk.co.real_logic.sbe.Tests.getLocalResource;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.parse;

class CompositeOffsetsTest
{
    @Test
    void shouldGenerateOffsetsForFieldsWithEmbeddedComposite() throws Exception
    {
        try (InputStream in = getLocalResource("composite-offsets-schema.xml"))
        {
            final MessageSchema schema = parse(in, ParserOptions.DEFAULT);
            final IrGenerator irg = new IrGenerator();

            final Ir ir = irg.generate(schema);
            final List<Token> message2 = ir.getMessage(2);

            final int fieldOneIndex = 1;
            assertThat(message2.get(fieldOneIndex).offset(), is(0));
            assertThat(message2.get(fieldOneIndex + 1).encodedLength(), is(4));

            final int fieldTwoIndex = 4;
            assertThat(message2.get(fieldTwoIndex).offset(), is(8));
            assertThat(message2.get(fieldTwoIndex + 1).encodedLength(), is(16));

            assertThat(message2.get(fieldTwoIndex + 2).offset(), is(0));
            assertThat(message2.get(fieldTwoIndex + 2).encodedLength(), is(1));
            assertThat(message2.get(fieldTwoIndex + 3).offset(), is(8));
            assertThat(message2.get(fieldTwoIndex + 3).encodedLength(), is(8));

            final int fieldThreeIndex = 10;
            assertThat(message2.get(fieldThreeIndex).offset(), is(24));
            assertThat(message2.get(fieldThreeIndex + 1).encodedLength(), is(8));

            assertThat(message2.get(0).encodedLength(), is(32));
        }
    }
}
