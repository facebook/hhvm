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

import org.junit.jupiter.api.Test;
import uk.co.real_logic.sbe.xml.IrGenerator;
import uk.co.real_logic.sbe.xml.MessageSchema;
import uk.co.real_logic.sbe.xml.ParserOptions;
import uk.co.real_logic.sbe.xml.XmlSchemaParser;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.PrintStream;
import java.nio.charset.StandardCharsets;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsString;
import static uk.co.real_logic.sbe.Tests.getLocalResource;

class WarningFormattingTest
{
    @Test
    void shouldGenerateCorrectWarning() throws Exception
    {
        try (InputStream in = getLocalResource("npe-small-header.xml"))
        {
            final ByteArrayOutputStream out = new ByteArrayOutputStream();
            final ParserOptions options = new ParserOptions.Builder()
                .stopOnError(true)
                .errorPrintStream(new PrintStream(out))
                .build();
            final MessageSchema schema = XmlSchemaParser.parse(in, options);
            final IrGenerator irg = new IrGenerator();
            irg.generate(schema);

            final String warnings = out.toString(StandardCharsets.US_ASCII.name());
            assertThat(warnings, containsString(
                "WARNING: at <types><composite name=\"messageHeader\"> \"blockLength\" should be UINT16"));
            assertThat(warnings, containsString(
                "WARNING: at <types><composite name=\"messageHeader\"> \"templateId\" should be UINT16"));
            assertThat(warnings, containsString(
                "WARNING: at <types><composite name=\"messageHeader\"> \"version\" should be UINT16"));
        }
    }
}
