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

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.core.Is.is;
import static uk.co.real_logic.sbe.Tests.getLocalResource;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.parse;

class ValueRefsTest
{
    @Test
    void shouldGenerateValueRefToEnum() throws Exception
    {
        try (InputStream in = getLocalResource("value-ref-schema.xml"))
        {
            final MessageSchema schema = parse(in, ParserOptions.DEFAULT);
            final IrGenerator irg = new IrGenerator();
            final Ir ir = irg.generate(schema);

            assertThat(ir.getMessage(1).get(1).encodedLength(), is(8));
            assertThat(ir.getMessage(2).get(1).encodedLength(), is(0));
            assertThat(ir.getMessage(3).get(1).encodedLength(), is(0));
            assertThat(ir.getMessage(4).get(1).encodedLength(), is(0));
            assertThat(ir.getMessage(5).get(1).encodedLength(), is(0));
        }
    }
}
