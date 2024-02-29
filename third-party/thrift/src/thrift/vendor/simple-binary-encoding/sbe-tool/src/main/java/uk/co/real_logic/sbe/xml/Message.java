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

import org.agrona.collections.IntHashSet;
import org.agrona.collections.ObjectHashSet;
import uk.co.real_logic.sbe.ir.Token;

import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import static javax.xml.xpath.XPathConstants.NODESET;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.*;

/**
 * An SBE message containing a list of {@link Field} objects and SBE message attributes.
 * <p>
 * What is difference between {@link Message} and the Intermediate Representation (IR)?
 * <ul>
 * <li>IR is intentionally platform, schema, and language independent.</li>
 * <li>IR is abstract layout and encoding only.</li>
 * <li>IR is a flat representation without cycles or hierarchy.</li>
 * <li>Message is FIX/SBE XML Schema specific.</li>
 * </ul>
 */
public class Message
{
    private static final String FIELD_OR_GROUP_OR_DATA_EXPR = "field|group|data";

    private final int id;
    private final String name;
    private final String description;
    private final int sinceVersion;
    private final int deprecated;
    private final int blockLength;
    private final List<Field> fieldList;
    private final String semanticType;
    private final int computedBlockLength;
    private final Map<String, Type> typeByNameMap;

    /**
     * Construct a new message from XML Schema.
     *
     * @param messageNode   from the XML Schema Parsing
     * @param typeByNameMap holding type information for message
     * @throws XPathExpressionException on invalid XPath
     */
    public Message(final Node messageNode, final Map<String, Type> typeByNameMap) throws XPathExpressionException
    {
        id = Integer.parseInt(getAttributeValue(messageNode, "id"));                        // required
        name = getAttributeValue(messageNode, "name");                                      // required
        description = getAttributeValueOrNull(messageNode, "description");                  // optional
        blockLength = Integer.parseInt(getAttributeValue(messageNode, "blockLength", "0")); // 0 means not set
        sinceVersion = Integer.parseInt(getAttributeValue(messageNode, "sinceVersion", "0"));
        deprecated = Integer.parseInt(getAttributeValue(messageNode, "deprecated", "0"));
        semanticType = getAttributeValueOrNull(messageNode, "semanticType");                // optional
        this.typeByNameMap = typeByNameMap;

        fieldList = parseMembers(messageNode);
        computeAndValidateOffsets(messageNode, fieldList, blockLength);

        computedBlockLength = computeMessageRootBlockLength(fieldList);
        validateBlockLength(messageNode, blockLength, computedBlockLength);
    }

    Message(final Message message, final List<Field> newFieldList)
    {
        id = message.id;
        name = message.name;
        description = message.description;
        blockLength = message.blockLength;
        sinceVersion = message.sinceVersion;
        deprecated = message.deprecated;
        semanticType = message.semanticType;
        typeByNameMap = message.typeByNameMap;
        fieldList = newFieldList;
        computedBlockLength = computeMessageRootBlockLength(newFieldList);
    }

    /**
     * Return the template schemaId of the message
     *
     * @return schemaId of the message
     */
    public int id()
    {
        return id;
    }

    /**
     * Return the name of the message
     *
     * @return name of the message
     */
    public String name()
    {
        return name;
    }

    /**
     * The description of the message (if set) or null
     *
     * @return description set by the message or null
     */
    public String description()
    {
        return description;
    }

    /**
     * The semanticType of the message (if set) or null
     *
     * @return the semanticType of the message (if set) or null
     */
    public String semanticType()
    {
        return semanticType;
    }

    /**
     * The version since this was added to the template.
     *
     * @return version since this was added to the template.
     */
    public int sinceVersion()
    {
        return sinceVersion;
    }

    /**
     * Version in which message was deprecated. Only valid if greater than zero.
     *
     * @return version in which the message was deprecated.
     */
    public int deprecated()
    {
        return deprecated;
    }

    /**
     * Return the list of fields in the message
     *
     * @return {@link java.util.List} of the Field objects in this Message
     */
    public List<Field> fields()
    {
        return fieldList;
    }

    /**
     * Return the encodedLength of the {@link Message} in bytes including any padding.
     *
     * @return the encodedLength of the {@link Message} in bytes including any padding.
     */
    public int blockLength()
    {
        return Math.max(blockLength, computedBlockLength);
    }

    private List<Field> parseMembers(final Node node) throws XPathExpressionException
    {
        final XPath xPath = XPathFactory.newInstance().newXPath();
        final NodeList list = (NodeList)xPath.compile(FIELD_OR_GROUP_OR_DATA_EXPR).evaluate(node, NODESET);
        boolean groupEncountered = false, dataEncountered = false;

        final ObjectHashSet<String> distinctNames = new ObjectHashSet<>();
        final IntHashSet distinctIds = new IntHashSet();
        final ArrayList<Field> fieldList = new ArrayList<>();

        for (int i = 0, size = list.getLength(); i < size; i++)
        {
            final Field field;
            final String nodeName = list.item(i).getNodeName();

            switch (nodeName)
            {
                case "group":
                    if (dataEncountered)
                    {
                        handleError(node, "group node specified after data node");
                    }

                    field = parseGroupField(list, i);
                    groupEncountered = true;
                    break;

                case "data":
                    field = parseDataField(list, i);
                    dataEncountered = true;
                    break;

                case "field":
                    if (groupEncountered || dataEncountered)
                    {
                        handleError(node, "field node specified after group or data node specified");
                    }

                    field = parseField(list, i);
                    break;

                default:
                    throw new IllegalStateException("Unknown node name: " + nodeName);
            }

            if (!distinctIds.add(field.id()))
            {
                handleError(node, "duplicate id found: " + field.id());
            }

            if (!distinctNames.add(field.name()))
            {
                handleError(node, "duplicate name found: " + field.name());
            }

            fieldList.add(field);
        }

        return fieldList;
    }

    private Field parseGroupField(final NodeList nodeList, final int nodeIndex) throws XPathExpressionException
    {
        final Node node = nodeList.item(nodeIndex);
        final String dimensionTypeName = getAttributeValue(node, "dimensionType", "groupSizeEncoding");
        Type dimensionType = typeByNameMap.get(dimensionTypeName);
        if (dimensionType == null)
        {
            handleError(node, "could not find dimensionType: " + dimensionTypeName);
        }
        else if (!(dimensionType instanceof CompositeType))
        {
            handleError(node, "dimensionType should be a composite type: " + dimensionTypeName);
            dimensionType = null;
        }
        else
        {
            ((CompositeType)dimensionType).checkForWellFormedGroupSizeEncoding(node);
        }

        final Field field = new Field.Builder()
            .name(getAttributeValue(node, "name"))
            .description(getAttributeValueOrNull(node, "description"))
            .id(Integer.parseInt(getAttributeValue(node, "id")))
            .blockLength(Integer.parseInt(getAttributeValue(node, "blockLength", "0")))
            .sinceVersion(Integer.parseInt(getAttributeValue(node, "sinceVersion", "0")))
            .deprecated(Integer.parseInt(getAttributeValue(node, "deprecated", "0")))
            .dimensionType((CompositeType)dimensionType)
            .build();

        XmlSchemaParser.checkForValidName(node, field.name());

        field.groupFields(parseMembers(node)); // recursive call

        return field;
    }

    private Field parseField(final NodeList nodeList, final int nodeIndex)
    {
        final Node node = nodeList.item(nodeIndex);
        final String typeName = getAttributeValue(node, "type");
        final Type fieldType = typeByNameMap.get(typeName);
        if (fieldType == null)
        {
            handleError(node, "could not find type: " + typeName);
        }

        final Field field = new Field.Builder()
            .name(getAttributeValue(node, "name"))
            .description(getAttributeValueOrNull(node, "description"))
            .id(Integer.parseInt(getAttributeValue(node, "id")))
            .offset(Integer.parseInt(getAttributeValue(node, "offset", "0")))
            .semanticType(getAttributeValueOrNull(node, "semanticType"))
            .presence(getPresence(node, fieldType))
            .valueRef(getAttributeValueOrNull(node, "valueRef"))
            .sinceVersion(Integer.parseInt(getAttributeValue(node, "sinceVersion", "0")))
            .deprecated(Integer.parseInt(getAttributeValue(node, "deprecated", "0")))
            .epoch(getAttributeValueOrNull(node, "epoch"))
            .timeUnit(getAttributeValueOrNull(node, "timeUnit"))
            .type(fieldType)
            .build();

        field.validate(node, typeByNameMap);

        return field;
    }

    private static Presence getPresence(final Node node, final Type fieldType)
    {
        final String presenceStr = getAttributeValueOrNull(node, "presence");
        final Presence presence;

        if (null != presenceStr)
        {
            presence = Presence.get(presenceStr);
        }
        else if (null != fieldType)
        {
            presence = fieldType.presence();
        }
        else
        {
            presence = Presence.REQUIRED;
        }

        return presence;
    }

    private Field parseDataField(final NodeList nodeList, final int nodeIndex)
    {
        final Node node = nodeList.item(nodeIndex);
        final String typeName = getAttributeValue(node, "type");
        final Type fieldType = typeByNameMap.get(typeName);
        if (fieldType == null)
        {
            handleError(node, "could not find type: " + typeName);
        }
        else if (!(fieldType instanceof CompositeType))
        {
            handleError(node, "data type is not composite type: " + typeName);
        }
        else
        {
            ((CompositeType)fieldType).checkForWellFormedVariableLengthDataEncoding(node);
            ((CompositeType)fieldType).makeDataFieldCompositeType();
        }

        final Field field = new Field.Builder()
            .name(getAttributeValue(node, "name"))
            .description(getAttributeValueOrNull(node, "description"))
            .id(Integer.parseInt(getAttributeValue(node, "id")))
            .offset(Integer.parseInt(getAttributeValue(node, "offset", "0")))
            .semanticType(getAttributeValueOrNull(node, "semanticType"))
            .presence(Presence.get(getAttributeValue(node, "presence", "required")))
            .sinceVersion(Integer.parseInt(getAttributeValue(node, "sinceVersion", "0")))
            .deprecated(Integer.parseInt(getAttributeValue(node, "deprecated", "0")))
            .epoch(getAttributeValueOrNull(node, "epoch"))
            .timeUnit(getAttributeValueOrNull(node, "timeUnit"))
            .type(fieldType)
            .variableLength(true)
            .build();

        field.validate(node, typeByNameMap);

        return field;
    }

    /*
     * Compute and validate the offsets of the fields in the list and will set the fields computedOffset.
     * Will validate the blockLength of the fields encompassing &lt;message&gt; or &lt;group&gt; and recursively
     * descend into repeated groups.
     */
    private static int computeAndValidateOffsets(final Node node, final List<Field> fields, final int blockLength)
    {
        boolean variableLengthBlock = false;
        int offset = 0;

        for (final Field field : fields)
        {
            if (0 != field.offset() && field.offset() < offset)
            {
                handleError(node, "Offset provides insufficient space at field: " + field.name());
            }

            if (Token.VARIABLE_LENGTH != offset)
            {
                if (0 != field.offset())
                {
                    offset = field.offset();
                }
                else if (null != field.dimensionType() && 0 != blockLength)
                {
                    offset = blockLength;
                }
                else if (field.isVariableLength() && 0 != blockLength)
                {
                    offset = blockLength;
                }
            }

            field.computedOffset(variableLengthBlock ? Token.VARIABLE_LENGTH : offset);

            if (null != field.groupFields())
            {
                final int groupBlockLength = computeAndValidateOffsets(node, field.groupFields(), 0);

                validateBlockLength(node, field.blockLength(), groupBlockLength);
                field.computedBlockLength(Math.max(field.blockLength(), groupBlockLength));

                variableLengthBlock = true;
            }
            else if (null != field.type() && Presence.CONSTANT != field.presence())
            {
                final int size = field.type().encodedLength();

                if (Token.VARIABLE_LENGTH == size)
                {
                    variableLengthBlock = true;
                }
                else
                {
                    field.computedBlockLength(size);
                }

                if (!variableLengthBlock)
                {
                    offset += size;
                }
            }
        }

        return offset;
    }

    private static int computeMessageRootBlockLength(final List<Field> fields)
    {
        int blockLength = 0;

        for (final Field field : fields)
        {
            if (field.groupFields() != null)
            {
                return blockLength;
            }
            else if (field.type() != null)
            {
                final int fieldLength = field.type().encodedLength();

                if (Token.VARIABLE_LENGTH == fieldLength)
                {
                    return blockLength;
                }

                if (field.presence() == Presence.CONSTANT)
                {
                    blockLength = field.computedOffset();
                }
                else
                {
                    blockLength = field.computedOffset() + fieldLength;
                }
            }
        }

        return blockLength;
    }

    private static void validateBlockLength(
        final Node node, final long specifiedBlockLength, final long computedBlockLength)
    {
        if (0 != specifiedBlockLength && computedBlockLength > specifiedBlockLength)
        {
            final String msg = "specified blockLength provides insufficient space " +
                computedBlockLength + " > " + specifiedBlockLength;

            handleError(node, msg);
        }
    }

    /**
     * {@inheritDoc}
     */
    public String toString()
    {
        return "Message{" +
            "id=" + id +
            ", name='" + name + '\'' +
            ", description='" + description + '\'' +
            ", sinceVersion=" + sinceVersion +
            ", deprecated=" + deprecated +
            ", blockLength=" + blockLength +
            ", fieldList=" + fieldList +
            ", semanticType='" + semanticType + '\'' +
            ", computedBlockLength=" + computedBlockLength +
            ", typeByNameMap=" + typeByNameMap +
            '}';
    }
}
