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
package uk.co.real_logic.sbe.generation.cpp;

import org.agrona.generation.StringWriterOutputManager;
import org.junit.jupiter.api.Test;
import uk.co.real_logic.sbe.Tests;
import uk.co.real_logic.sbe.ir.Ir;
import uk.co.real_logic.sbe.xml.IrGenerator;
import uk.co.real_logic.sbe.xml.MessageSchema;
import uk.co.real_logic.sbe.xml.ParserOptions;

import java.io.InputStream;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.not;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.parse;

class CppGeneratorTest
{
    @Test
    void shouldUseGeneratedLiteralForConstantOneWhenGeneratingBitsetCode() throws Exception
    {
        try (InputStream in = Tests.getLocalResource("issue827.xml"))
        {
            final ParserOptions options = ParserOptions.builder().stopOnError(true).build();
            final MessageSchema schema = parse(in, options);
            final IrGenerator irg = new IrGenerator();
            final Ir ir = irg.generate(schema);
            final StringWriterOutputManager outputManager = new StringWriterOutputManager();
            outputManager.setPackageName(ir.applicableNamespace());

            final CppGenerator generator = new CppGenerator(ir, false, outputManager);
            generator.generate();

            final String source = outputManager.getSource("issue827.FlagsSet").toString();
            assertThat(source, not(containsString("1u << ")));
            assertThat(source, containsString("UINT64_C(0x1) << "));
        }
    }

    @Test
    void shouldUseConstexprWhenInitializingSemanticVersion() throws Exception
    {
        try (InputStream in = Tests.getLocalResource("code-generation-schema.xml"))
        {
            final ParserOptions options = ParserOptions.builder().stopOnError(true).build();
            final MessageSchema schema = parse(in, options);
            final IrGenerator irg = new IrGenerator();
            final Ir ir = irg.generate(schema);
            final StringWriterOutputManager outputManager = new StringWriterOutputManager();
            outputManager.setPackageName(ir.applicableNamespace());

            final CppGenerator generator = new CppGenerator(ir, false, outputManager);
            generator.generate();

            final String source = outputManager.getSource("code.generation.test.Car").toString();
            assertThat(source, containsString("static constexpr const char* SBE_SEMANTIC_VERSION = \"5.2\""));
        }
    }
}
