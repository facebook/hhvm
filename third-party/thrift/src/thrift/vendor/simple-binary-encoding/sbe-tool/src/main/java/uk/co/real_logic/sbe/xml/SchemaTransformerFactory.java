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

import java.util.HashMap;
import java.util.Map;

/**
 * Entry point for schema transformations, will check all incoming configuration/properties and create the appropriate
 * transformers as required.
 */
public class SchemaTransformerFactory implements SchemaTransformer
{
    private final SchemaTransformer transformer;

    /**
     * Construct the SchemaTransformerFactory with the specified configuration for filtering the messages and types by
     * version.
     *
     * @param schemaTransformConfig configuration for the sinceVersion transformation
     * @see uk.co.real_logic.sbe.SbeTool#SCHEMA_TRANSFORM_VERSION
     */
    public SchemaTransformerFactory(final String schemaTransformConfig)
    {
        transformer = parse(schemaTransformConfig);
    }

    /**
     * {@inheritDoc}
     */
    public MessageSchema transform(final MessageSchema originalSchema)
    {
        return transformer.transform(originalSchema);
    }

    static SchemaTransformer parse(final String configuration)
    {
        if (null == configuration || configuration.isEmpty())
        {
            return IDENTITY_TRANSFORMER;
        }

        final String[] split = configuration.split(",");
        if (0 == split.length)
        {
            return IDENTITY_TRANSFORMER;
        }

        final HashMap<Integer, SchemaTransformer> transformerBySchemaId = new HashMap<>();
        parseComponents(split, transformerBySchemaId);

        SchemaTransformer defaultTransformer = transformerBySchemaId.remove(-1);
        defaultTransformer = null != defaultTransformer ? defaultTransformer : IDENTITY_TRANSFORMER;

        return transformerBySchemaId.isEmpty() ?
            defaultTransformer : new IdKeyedSchemaTransformer(transformerBySchemaId, defaultTransformer);
    }

    private static void parseComponents(
        final String[] configuration,
        final Map<Integer, SchemaTransformer> transformerBySchemaId)
    {
        for (final String field : configuration)
        {
            final String[] fieldParts = field.split(":");

            if (2 != fieldParts.length)
            {
                throw new IllegalArgumentException("version transformation property part '" + field + "' is invalid");
            }

            final int schemaId = "*".equals(fieldParts[0]) ? -1 : Integer.parseInt(fieldParts[0].trim());
            final int sinceVersion = Integer.parseInt(fieldParts[1].trim());
            transformerBySchemaId.put(schemaId, new SinceVersionSchemaTransformer(sinceVersion));
        }
    }

    SchemaTransformer delegate()
    {
        return transformer;
    }
}
