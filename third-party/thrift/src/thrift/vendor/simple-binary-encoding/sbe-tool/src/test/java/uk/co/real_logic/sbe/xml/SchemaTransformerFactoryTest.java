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

import static org.junit.jupiter.api.Assertions.*;

class SchemaTransformerFactoryTest
{
    @Test
    void shouldAcceptNullAsTransformConfiguration()
    {
        final SchemaTransformerFactory schemaTransformerFactory = new SchemaTransformerFactory(null);
        assertSame(SchemaTransformer.IDENTITY_TRANSFORMER, schemaTransformerFactory.delegate());
    }

    @Test
    void shouldAcceptEmptyStringAsTransformConfiguration()
    {
        final SchemaTransformerFactory schemaTransformerFactory = new SchemaTransformerFactory("");
        assertSame(SchemaTransformer.IDENTITY_TRANSFORMER, schemaTransformerFactory.delegate());
    }

    @Test
    void shouldAllSchemasAsOnlyInput()
    {
        final SchemaTransformerFactory schemaTransformerFactory = new SchemaTransformerFactory("*:5");
        final SchemaTransformer delegate = schemaTransformerFactory.delegate();
        assertInstanceOf(SinceVersionSchemaTransformer.class, delegate);

        final SinceVersionSchemaTransformer transformer = (SinceVersionSchemaTransformer)delegate;
        assertEquals(5, transformer.sinceVersion());
    }

    @Test
    void shouldHandleMultiSchemaTransformation()
    {
        final SchemaTransformerFactory schemaTransformerFactory = new SchemaTransformerFactory("1:20,4:32,*:5");
        final SchemaTransformer delegate = schemaTransformerFactory.delegate();
        assertInstanceOf(IdKeyedSchemaTransformer.class, delegate);

        final IdKeyedSchemaTransformer transformer = (IdKeyedSchemaTransformer)delegate;
        final SinceVersionSchemaTransformer schemaTransformer1 =
            (SinceVersionSchemaTransformer)transformer.lookupTransformer(1);
        assertEquals(20, schemaTransformer1.sinceVersion());
        final SinceVersionSchemaTransformer schemaTransformer4 =
            (SinceVersionSchemaTransformer)transformer.lookupTransformer(4);
        assertEquals(32, schemaTransformer4.sinceVersion());
        final SinceVersionSchemaTransformer schemaTransformerDefault =
            (SinceVersionSchemaTransformer)transformer.lookupTransformer(89732465);
        assertEquals(5, schemaTransformerDefault.sinceVersion());
    }
}