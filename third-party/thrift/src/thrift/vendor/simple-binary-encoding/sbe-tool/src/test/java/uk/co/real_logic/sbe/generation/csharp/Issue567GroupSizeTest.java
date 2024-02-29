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
package uk.co.real_logic.sbe.generation.csharp;

import org.agrona.generation.StringWriterOutputManager;
import org.junit.jupiter.api.Test;
import org.xml.sax.InputSource;
import uk.co.real_logic.sbe.Tests;
import uk.co.real_logic.sbe.ir.Ir;
import uk.co.real_logic.sbe.xml.IrGenerator;
import uk.co.real_logic.sbe.xml.MessageSchema;
import uk.co.real_logic.sbe.xml.ParserOptions;
import uk.co.real_logic.sbe.xml.XmlSchemaParser;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.nio.charset.StandardCharsets;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsString;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;

class Issue567GroupSizeTest
{
    private static final String ERR_MSG =
        "WARNING: at <sbe:message name=\"issue567\"> <group name=\"group\"> \"numInGroup\" should be UINT8 or UINT16";

    private static final String EXPECTED_COUNT_CHECK =
        "                if ((uint) count > 2147483647)\n" +
        "                {\n" +
        "                    ThrowHelper.ThrowCountOutOfRangeException(count);\n" +
        "                }";

    private final PrintStream mockErr = mock(PrintStream.class);

    @Test
    void shouldThrowWhenUsingATypeThatIsNotConstrainedToFitInAnIntAsTheGroupSize() throws IOException
    {
        final ParserOptions options = ParserOptions.builder()
            .errorPrintStream(mockErr)
            .stopOnError(true)
            .build();
        final InputStream in = Tests.getLocalResource("issue567-invalid.xml");

        try (InputStreamReader characterStream = new InputStreamReader(in, StandardCharsets.UTF_8))
        {
            final InputSource is = new InputSource(characterStream);

            assertThrows(IllegalArgumentException.class, () -> XmlSchemaParser.parse(is, options));
            verify(mockErr).println(ERR_MSG);
        }
    }

    @Test
    void shouldGenerateWhenUsingATypeThatIsConstrainedToFitInAnIntAsTheGroupSize() throws Exception
    {
        final ParserOptions options = ParserOptions.builder()
            .errorPrintStream(mockErr)
            .stopOnError(true)
            .build();
        final InputStream in = Tests.getLocalResource("issue567-valid.xml");

        try (InputStreamReader characterStream = new InputStreamReader(in, StandardCharsets.UTF_8))
        {
            final InputSource is = new InputSource(characterStream);

            final MessageSchema schema = XmlSchemaParser.parse(is, options);
            verify(mockErr).println(ERR_MSG);

            final IrGenerator irg = new IrGenerator();
            final Ir ir = irg.generate(schema);

            final StringWriterOutputManager outputManager = new StringWriterOutputManager();
            outputManager.setPackageName(ir.applicableNamespace());
            final CSharpGenerator generator = new CSharpGenerator(ir, outputManager);

            generator.generate();

            final String source = outputManager.getSource("tests.Issue567").toString();
            assertThat(source, containsString(EXPECTED_COUNT_CHECK));
        }
    }
}
