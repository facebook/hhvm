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

import org.agrona.Verify;
import org.w3c.dom.Node;

import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;
import java.nio.ByteOrder;
import java.util.ArrayDeque;
import java.util.Collection;
import java.util.Deque;
import java.util.List;
import java.util.Map;

import static uk.co.real_logic.sbe.xml.XmlSchemaParser.getAttributeValue;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.getAttributeValueOrNull;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.getByteOrder;

/**
 * Message schema aggregate for schema attributes, messageHeader, and reference for multiple {@link Message} objects.
 */
public class MessageSchema
{
    /**
     * Default message header type name for the SBE spec.
     */
    public static final String HEADER_TYPE_DEFAULT = "messageHeader";

    private final String packageName;                 // package (required)
    private final String description;                 // description (optional)
    private final int id;                             // identifier for the schema (required)
    private final int version;                        // version (optional - default is 0)
    private final String semanticVersion;             // semanticVersion (optional)
    private final ByteOrder byteOrder;                // byteOrder (optional - default is littleEndian)
    private final String headerType;                  // headerType (optional - default to messageHeader)
    private final Map<String, Type> typeByNameMap;
    private final Map<Long, Message> messageByIdMap;

    MessageSchema(final Node schemaNode, final Map<String, Type> typeByNameMap, final Map<Long, Message> messageByIdMap)
    {
        this.packageName = getAttributeValue(schemaNode, "package");
        this.description = getAttributeValueOrNull(schemaNode, "description");
        this.id = Integer.parseInt(getAttributeValue(schemaNode, "id"));
        this.version = Integer.parseInt(getAttributeValue(schemaNode, "version", "0"));
        this.semanticVersion = getAttributeValueOrNull(schemaNode, "semanticVersion");
        this.byteOrder = getByteOrder(getAttributeValue(schemaNode, "byteOrder", "littleEndian"));
        this.typeByNameMap = typeByNameMap;
        this.messageByIdMap = messageByIdMap;

        final String configuredHeaderType = getAttributeValueOrNull(schemaNode, "headerType");
        headerType = null == configuredHeaderType ? HEADER_TYPE_DEFAULT : configuredHeaderType;
        Verify.present(typeByNameMap, this.headerType, "Message header");

        final Node messageHeaderNode = findNode(schemaNode, "types/composite[@name='" + this.headerType + "']");
        ((CompositeType)typeByNameMap.get(this.headerType)).checkForWellFormedMessageHeader(messageHeaderNode);
    }

    MessageSchema(
        final String packageName,
        final String description,
        final int id,
        final int version,
        final String semanticVersion,
        final ByteOrder byteOrder,
        final String headerType,
        final Map<String, Type> typeByNameMap,
        final Map<Long, Message> messageByIdMap)
    {
        this.packageName = packageName;
        this.description = description;
        this.id = id;
        this.version = version;
        this.semanticVersion = semanticVersion;
        this.byteOrder = byteOrder;
        this.headerType = headerType;
        this.typeByNameMap = typeByNameMap;
        this.messageByIdMap = messageByIdMap;
    }

    /**
     * The Schema headerType for message headers. This should be a {@link CompositeType}.
     *
     * @return the Schema headerType for message headers
     */
    public CompositeType messageHeader()
    {
        return (CompositeType)typeByNameMap.get(headerType);
    }

    /**
     * The package name for the schema.
     *
     * @return he package name for the schema.
     */
    public String packageName()
    {
        return packageName;
    }

    /**
     * The description of the schema.
     *
     * @return the description of the schema.
     */
    public String description()
    {
        return description;
    }

    /**
     * The id number of the schema.
     *
     * @return the id number of the schema.
     */
    public int id()
    {
        return id;
    }

    /**
     * The version number of the schema.
     *
     * @return the version number of the schema.
     */
    public int version()
    {
        return version;
    }

    /**
     * The semantic version number of the schema. Typically, used to reference a third party standard such as FIX.
     *
     * @return the semantic version number of the schema.
     */
    public String semanticVersion()
    {
        return semanticVersion;
    }

    /**
     * Return a given {@link Message} object with the given messageId.
     *
     * @param messageId of the message to return.
     * @return a given {@link Message} for the messageId.
     */
    public Message getMessage(final long messageId)
    {
        return messageByIdMap.get(messageId);
    }

    /**
     * Get the {@link Type} for a given name.
     *
     * @param typeName to lookup.
     * @return the type if found otherwise null.
     */
    public Type getType(final String typeName)
    {
        return typeByNameMap.get(typeName);
    }

    /**
     * Get the {@link Collection} of {@link Message}s for this Schema.
     *
     * @return the {@link Collection} of {@link Message}s for this Schema.
     */
    public Collection<Message> messages()
    {
        return messageByIdMap.values();
    }

    /**
     * Get the {@link Collection} of {@link Type}s for this Schema.
     *
     * @return the {@link Collection} of {@link Type}s for this Schema.
     */
    public Collection<Type> types()
    {
        return typeByNameMap.values();
    }

    /**
     * Return the byte order specified by the messageSchema
     *
     * @return {@link ByteOrder} of the message encoding.
     */
    public ByteOrder byteOrder()
    {
        return byteOrder;
    }

    /**
     * Validate the message schema and delegate warnings and errors to the supplied {@link ErrorHandler}.
     *
     * @param errorHandler for delegating warnings and errors.
     */
    public void validate(final ErrorHandler errorHandler)
    {
        final Deque<String> path = new ArrayDeque<>();

        for (final Type type : typeByNameMap.values())
        {
            validateType(errorHandler, path, type);
        }

        for (final Message message : messageByIdMap.values())
        {
            if (message.sinceVersion() > version)
            {
                errorHandler.error(message.name() + ".sinceVersion=" + message.sinceVersion() +
                    " > messageSchema.version=" + version);
            }

            path.addLast(message.name());

            for (final Field field : message.fields())
            {
                validateField(errorHandler, path, field);
            }

            path.removeLast();
        }
    }

    private void validateType(final ErrorHandler errorHandler, final Deque<String> path, final Type type)
    {
        if (type instanceof EncodedDataType)
        {
            validateEncodedType(errorHandler, path, (EncodedDataType)type);
        }
        else if (type instanceof EnumType)
        {
            validateEnumType(errorHandler, path, (EnumType)type);
        }
        else if (type instanceof SetType)
        {
            validateSetType(errorHandler, path, (SetType)type);
        }
        else if (type instanceof CompositeType)
        {
            validateCompositeType(errorHandler, path, (CompositeType)type);
        }
    }

    private void validateEncodedType(
        final ErrorHandler errorHandler, final Deque<String> path, final EncodedDataType type)
    {
        if (type.sinceVersion() > version)
        {
            reportError(errorHandler, path, type.name(), type.sinceVersion());
        }
    }

    private void validateEnumType(final ErrorHandler errorHandler, final Deque<String> path, final EnumType type)
    {
        if (type.sinceVersion() > version)
        {
            reportError(errorHandler, path, type.name(), type.sinceVersion());
        }

        path.addLast(type.name());

        for (final EnumType.ValidValue validValue : type.validValues())
        {
            if (validValue.sinceVersion() > version)
            {
                reportError(errorHandler, path, validValue.name(), validValue.sinceVersion());
            }
        }

        path.removeLast();
    }

    private void validateSetType(final ErrorHandler errorHandler, final Deque<String> path, final SetType type)
    {
        if (type.sinceVersion() > version)
        {
            reportError(errorHandler, path, type.name(), type.sinceVersion());
        }

        path.addLast(type.name());

        for (final SetType.Choice choice : type.choices())
        {
            if (choice.sinceVersion() > version)
            {
                reportError(errorHandler, path, choice.name(), choice.sinceVersion());
            }
        }

        path.removeLast();
    }

    private void validateCompositeType(
        final ErrorHandler errorHandler, final Deque<String> path, final CompositeType type)
    {
        if (type.sinceVersion() > version)
        {
            reportError(errorHandler, path, type.name(), type.sinceVersion());
        }

        path.addLast(type.name());

        for (final Type subType : type.getTypeList())
        {
            validateType(errorHandler, path, subType);
        }

        path.removeLast();
    }

    private void validateField(final ErrorHandler errorHandler, final Deque<String> path, final Field field)
    {
        if (field.sinceVersion() > version)
        {
            reportError(errorHandler, path, field.name(), field.sinceVersion());
        }

        final List<Field> groupFields = field.groupFields();
        if (null != groupFields)
        {
            path.addLast(field.name());

            for (final Field groupField : groupFields)
            {
                validateField(errorHandler, path, groupField);
            }

            path.removeLast();
        }
    }

    private void reportError(
        final ErrorHandler errorHandler, final Deque<String> path, final String name, final int sinceVersion)
    {
        final StringBuilder sb = new StringBuilder();

        for (final String step : path)
        {
            sb.append(step).append('.');
        }

        sb.append(name)
            .append(".sinceVersion=").append(sinceVersion)
            .append(" > messageSchema.version=").append(version);

        errorHandler.error(sb.toString());
    }

    private static Node findNode(final Node contextNode, final String path)
    {
        try
        {
            return (Node)XPathFactory.newInstance().newXPath()
                .evaluate(path, contextNode, XPathConstants.NODE);
        }
        catch (final XPathExpressionException ex)
        {
            throw new IllegalArgumentException("Unable to locate node with path=" + path, ex);
        }
    }
}
