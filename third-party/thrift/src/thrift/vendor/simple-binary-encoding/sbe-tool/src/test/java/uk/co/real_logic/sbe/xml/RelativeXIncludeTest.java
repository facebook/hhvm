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
package uk.co.real_logic.sbe.xml;

import org.junit.jupiter.api.Test;
import org.xml.sax.InputSource;

import java.io.File;
import java.net.URL;

import static org.junit.jupiter.api.Assertions.assertNotNull;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.parse;

class RelativeXIncludeTest
{
    @Test
    void shouldParseFileInSubDir() throws Exception
    {
        final URL testResource = getClass().getClassLoader().getResource("sub/basic-schema.xml");
        assertNotNull(testResource);

        final InputSource is = new InputSource(testResource.openStream());
        final File file = new File(testResource.getFile());
        is.setSystemId(file.toPath().toAbsolutePath().getParent().toUri().toString());
        final MessageSchema messageSchema = parse(is, ParserOptions.DEFAULT);

        assertNotNull(messageSchema.getType("Symbol"));
    }
}