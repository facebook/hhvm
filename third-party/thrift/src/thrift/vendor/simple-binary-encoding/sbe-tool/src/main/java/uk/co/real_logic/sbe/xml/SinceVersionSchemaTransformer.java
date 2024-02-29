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

import java.util.*;

class SinceVersionSchemaTransformer implements SchemaTransformer
{
    private final int sinceVersion;

    SinceVersionSchemaTransformer(final int sinceVersion)
    {
        this.sinceVersion = sinceVersion;
    }

    /**
     * {@inheritDoc}
     */
    public MessageSchema transform(final MessageSchema originalSchema)
    {
        final Collection<Type> types = originalSchema.types();
        final Map<String, Type> newTypes = new HashMap<>();

        for (final Type type : types)
        {
            if (type.sinceVersion() <= this.sinceVersion)
            {
                newTypes.put(type.name(), type);
            }
        }

        final Collection<Message> messages = originalSchema.messages();
        final Map<Long, Message> newMessages = new HashMap<>();
        for (final Message message : messages)
        {
            final List<Field> newFields = removeFields(message.fields(), sinceVersion);
            final Message newMessage = new Message(message, newFields);

            if (newMessage.sinceVersion() <= this.sinceVersion)
            {
                newMessages.put((long)newMessage.id(), newMessage);
            }
        }

        return new MessageSchema(
            originalSchema.packageName(),
            originalSchema.description(),
            originalSchema.id(),
            sinceVersion,
            originalSchema.semanticVersion(),
            originalSchema.byteOrder(),
            originalSchema.messageHeader().name(),
            newTypes,
            newMessages);
    }

    private List<Field> removeFields(final List<Field> fields, final int sinceVersion)
    {
        final ArrayList<Field> newFields = new ArrayList<>();
        for (final Field field : fields)
        {
            if (field.sinceVersion() > sinceVersion)
            {
                continue;
            }

            Field newField = field;
            List<Field> groupFields = field.groupFields();
            if (null != groupFields && !groupFields.isEmpty())
            {
                groupFields = removeFields(groupFields, sinceVersion);
                newField = new Field(field, groupFields);
            }

            newFields.add(newField);
        }

        return newFields;
    }

    int sinceVersion()
    {
        return sinceVersion;
    }
}
