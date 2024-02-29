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
import uk.co.real_logic.sbe.Tests;

import java.io.InputStream;
import java.util.List;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.core.Is.is;
import static org.junit.jupiter.api.Assertions.assertTrue;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.parse;

class GroupWithDataTest
{
    @Test
    void shouldParseSchemaSuccessfully() throws Exception
    {
        try (InputStream in = Tests.getLocalResource("group-with-data-schema.xml"))
        {
            final MessageSchema schema = parse(in, ParserOptions.DEFAULT);
            final List<Field> fields = schema.getMessage(1).fields();
            final Field entriesGroup = fields.get(1);
            final CompositeType dimensionType = entriesGroup.dimensionType();
            final List<Field> entriesFields = entriesGroup.groupFields();

            assertThat(entriesGroup.name(), is("Entries"));
            assertThat(dimensionType.name(), is("groupSizeEncoding"));

            final Field varDataField = entriesFields.get(2);
            assertThat(varDataField.name(), is("varDataField"));
            assertTrue(varDataField.isVariableLength());
        }
    }
}
