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

import java.util.Map;

class IdKeyedSchemaTransformer implements SchemaTransformer
{
    private final Map<Integer, SchemaTransformer> transformerBySchemaId;
    private final SchemaTransformer defaultTransformer;

    IdKeyedSchemaTransformer(
        final Map<Integer, SchemaTransformer> transformerBySchemaId,
        final SchemaTransformer defaultTransformer)
    {
        this.transformerBySchemaId = transformerBySchemaId;
        this.defaultTransformer = defaultTransformer;
    }

    public MessageSchema transform(final MessageSchema originalSchema)
    {
        return lookupTransformer(originalSchema.id()).transform(originalSchema);
    }

    SchemaTransformer lookupTransformer(final int schemaId)
    {
        return transformerBySchemaId.getOrDefault(schemaId, defaultTransformer);
    }
}
