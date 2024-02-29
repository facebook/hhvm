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
import org.agrona.generation.StringWriterOutputManager;
import org.junit.jupiter.api.Test;
import uk.co.real_logic.sbe.Tests;
import uk.co.real_logic.sbe.ir.Ir;
import uk.co.real_logic.sbe.xml.IrGenerator;
import uk.co.real_logic.sbe.xml.MessageSchema;
import uk.co.real_logic.sbe.xml.ParserOptions;
import uk.co.real_logic.sbe.xml.XmlSchemaParser;

import java.io.InputStream;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.core.StringContains.containsString;

class ConstantCharArrayTest
{
    private static final Class<?> BUFFER_CLASS = MutableDirectBuffer.class;
    private static final String BUFFER_NAME = BUFFER_CLASS.getName();
    private static final Class<DirectBuffer> READ_ONLY_BUFFER_CLASS = DirectBuffer.class;
    private static final String READ_ONLY_BUFFER_NAME = READ_ONLY_BUFFER_CLASS.getName();

    @Test
    void shouldGenerateConstCharArrayMethods() throws Exception
    {
        try (InputStream in = Tests.getLocalResource("issue505.xml"))
        {
            final ParserOptions options = ParserOptions.builder().stopOnError(true).build();
            final MessageSchema schema = XmlSchemaParser.parse(in, options);
            final IrGenerator irg = new IrGenerator();
            final Ir ir = irg.generate(schema);

            final StringWriterOutputManager outputManager = new StringWriterOutputManager();
            outputManager.setPackageName(ir.applicableNamespace());
            final JavaGenerator generator = new JavaGenerator(
                ir, BUFFER_NAME, READ_ONLY_BUFFER_NAME, false, false, false, outputManager);

            generator.generate();
            final String sources = outputManager.getSources().toString();

            final String expectedOne =
                "    public byte sourceOne()\n" +
                "    {\n" +
                "        return (byte)67;\n" +
                "    }";
            assertThat(sources, containsString(expectedOne));

            final String expectedTwo =
                "    public byte sourceTwo()\n" +
                "    {\n" +
                "        return (byte)68;\n" +
                "    }";
            assertThat(sources, containsString(expectedTwo));

            final String expectedThree =
                "    public String sourceThree()\n" +
                "    {\n" +
                "        return \"EF\";\n" +
                "    }";
            assertThat(sources, containsString(expectedThree));

            final String expectedFour =
                "    public String sourceFour()\n" +
                "    {\n" +
                "        return \"GH\";\n" +
                "    }";
            assertThat(sources, containsString(expectedFour));
        }
    }
}
