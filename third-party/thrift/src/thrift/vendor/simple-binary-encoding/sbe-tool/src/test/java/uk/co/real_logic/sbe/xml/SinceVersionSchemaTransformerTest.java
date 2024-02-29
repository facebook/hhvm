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

import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.ValueSource;
import uk.co.real_logic.sbe.Tests;

import java.io.InputStream;
import java.util.*;

import static java.util.Comparator.comparing;
import static org.junit.jupiter.api.Assertions.*;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.parse;

public class SinceVersionSchemaTransformerTest
{
    @ParameterizedTest
    @ValueSource(ints = { 0, 4, 5 })
    void shouldFilterAllVersionedFields(final int filteringVersion) throws Exception
    {
        try (InputStream in = Tests.getLocalResource("since-version-filter-schema.xml"))
        {
            final MessageSchema schema = parse(in, ParserOptions.DEFAULT);

            final SinceVersionSchemaTransformer sinceVersionSchemaTransformer = new SinceVersionSchemaTransformer(
                filteringVersion);
            final MessageSchema transformedSchema = sinceVersionSchemaTransformer.transform(schema);

            assertEquals(filteringVersion, transformedSchema.version());
            assertTypeSinceVersionLessOrEqualTo(filteringVersion, schema, transformedSchema);
            assertMessageSinceVersionLessOrEqualTo(filteringVersion, schema, transformedSchema);
        }
    }

    private static void assertMessageSinceVersionLessOrEqualTo(
        final int filteringVersion,
        final MessageSchema originalSchema,
        final MessageSchema transformedSchema)
    {
        final ArrayList<Message> transformedMessagesCopy = new ArrayList<>(transformedSchema.messages());

        final Collection<Message> messages = originalSchema.messages();
        for (final Message originalMessage : messages)
        {

            if (originalMessage.sinceVersion() <= filteringVersion)
            {
                final Message transformedMessage = findAndRemove(
                    transformedMessagesCopy, originalMessage, comparing(Message::id));
                assertNotNull(transformedMessage, "Message (" + originalMessage.name() + ") should be retained");

                assertFieldsSinceVersionLessOrEqualTo(
                    filteringVersion, originalMessage.fields(), transformedMessage.fields());
            }
            else
            {
                assertNull(
                    findAndRemove(transformedMessagesCopy, originalMessage, comparing(Message::id)),
                    "Message (" + originalMessage.name() + ") should be removed");
            }
        }

        assertTrue(transformedMessagesCopy.isEmpty(), "Messages should have been removed: " + transformedMessagesCopy);
    }

    private static void assertFieldsSinceVersionLessOrEqualTo(
        final int filteringVersion,
        final List<Field> originalFields,
        final List<Field> transformedFields)
    {
        assertFalse(null == originalFields ^ null == transformedFields);
        if (null == originalFields)
        {
            return;
        }

        final ArrayList<Field> transformedFieldsCopy = new ArrayList<>(transformedFields);

        for (final Field originalField : originalFields)
        {
            if (originalField.sinceVersion() <= filteringVersion)
            {
                final Field transformedField = findAndRemove(
                    transformedFieldsCopy, originalField, comparing(Field::name));
                assertNotNull(transformedField, "Field (" + originalField.name() + ") should be retained");

                assertFieldsSinceVersionLessOrEqualTo(
                    filteringVersion, originalField.groupFields(), transformedField.groupFields());
            }
            else
            {
                assertNull(
                    findAndRemove(transformedFieldsCopy, originalField, comparing(Field::name)),
                    "Field (" + originalField.name() + ") should be removed");
            }
        }

        assertTrue(transformedFieldsCopy.isEmpty(), "Fields should have been removed: " + transformedFields);
    }

    private static void assertTypeSinceVersionLessOrEqualTo(
        final int filteringVersion,
        final MessageSchema originalSchema,
        final MessageSchema transformedSchema)
    {
        final ArrayList<Type> transformedTypesCopy = new ArrayList<>(transformedSchema.types());

        final Collection<Type> types = originalSchema.types();
        for (final Type type : types)
        {
            if (type.sinceVersion() <= filteringVersion)
            {
                assertNotNull(
                    findAndRemove(transformedTypesCopy, type, comparing(Type::name)),
                    "Type (" + type.name() + ") should be retained");
            }
            else
            {
                assertNull(
                    findAndRemove(transformedTypesCopy, type, comparing(Type::name)),
                    "Type (" + type.name() + ") should be removed");
            }
        }

        assertTrue(transformedTypesCopy.isEmpty(), "Types should have been removed: " + transformedTypesCopy);
    }

    private static <T> T findAndRemove(
        final ArrayList<T> transformedTsCopy,
        final T original,
        final Comparator<T> comparator)
    {
        T result = null;
        for (final Iterator<T> it = transformedTsCopy.iterator(); it.hasNext();)
        {
            final T transformedT = it.next();
            if (0 == comparator.compare(original, transformedT))
            {
                result = transformedT;
                it.remove();
            }
        }

        return result;
    }
}
