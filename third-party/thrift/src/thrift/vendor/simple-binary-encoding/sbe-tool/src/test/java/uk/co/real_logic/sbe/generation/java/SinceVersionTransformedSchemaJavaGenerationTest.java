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

import org.agrona.generation.StringWriterOutputManager;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.CsvSource;
import uk.co.real_logic.sbe.Tests;
import uk.co.real_logic.sbe.ir.Ir;
import uk.co.real_logic.sbe.xml.*;

import java.io.InputStream;
import java.util.Map;

import static org.junit.jupiter.api.Assertions.assertTrue;
import static uk.co.real_logic.sbe.SbeTool.JAVA_DEFAULT_DECODING_BUFFER_TYPE;
import static uk.co.real_logic.sbe.SbeTool.JAVA_DEFAULT_ENCODING_BUFFER_TYPE;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.parse;

public class SinceVersionTransformedSchemaJavaGenerationTest
{
    @ParameterizedTest
    @CsvSource({ "0,4", "4,5", "5,6" })
    void shouldGenerateCodeForOlderVersion(final int versionIncluded, final int versionExcluded) throws Exception
    {
        try (InputStream in = Tests.getLocalResource("since-version-filter-schema.xml"))
        {
            final MessageSchema schema = parse(in, ParserOptions.DEFAULT);
            final SchemaTransformer transformer = new SchemaTransformerFactory("*:" + versionIncluded);
            final MessageSchema transformedSchema = transformer.transform(schema);
            final Ir ir = new IrGenerator().generate(transformedSchema);
            final StringWriterOutputManager outputManager = new StringWriterOutputManager();
            outputManager.setPackageName("test");

            final JavaGenerator javaGenerator = new JavaGenerator(
                ir,
                JAVA_DEFAULT_ENCODING_BUFFER_TYPE,
                JAVA_DEFAULT_DECODING_BUFFER_TYPE,
                false,
                false,
                false,
                outputManager);

            javaGenerator.generate();

            final Map<String, CharSequence> sources = outputManager.getSources();
            assertTrue(containsCodeWithSinceVersion(sources, versionIncluded));
            assertTrue(doesNotContainsCodeWithSinceVersion(sources, versionExcluded));
        }
    }

    private static boolean containsCodeWithSinceVersion(final Map<String, CharSequence> sources, final int version)
    {
        final String nameFragment = "Since" + version;
        return sources.keySet().stream().anyMatch(s -> s.contains(nameFragment));
    }

    private boolean doesNotContainsCodeWithSinceVersion(final Map<String, CharSequence> sources, final int version)
    {
        final String nameFragment = "Since" + version;
        return sources.keySet().stream().noneMatch(s -> s.contains(nameFragment));
    }
}
