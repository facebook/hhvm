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

import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import uk.co.real_logic.sbe.PrimitiveType;
import uk.co.real_logic.sbe.PrimitiveValue;
import uk.co.real_logic.sbe.ir.Token;

import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import static javax.xml.xpath.XPathConstants.NODESET;
import static uk.co.real_logic.sbe.PrimitiveType.*;
import static uk.co.real_logic.sbe.SbeTool.JAVA_GENERATE_INTERFACES;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.getAttributeValue;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.getAttributeValueOrNull;

/**
 * SBE compositeType which is a composite of other composites, sets, enums, or simple types.
 */
public class CompositeType extends Type
{
    /**
     * SBE schema composite type.
     */
    public static final String COMPOSITE_TYPE = "composite";
    private static final String SUB_TYPES_EXP = "type|enum|set|composite|ref|data|group";

    private final List<String> compositesPath = new ArrayList<>();
    private final Map<String, Type> containedTypeByNameMap = new LinkedHashMap<>();

    CompositeType(final Node node) throws XPathExpressionException
    {
        this(node, null, null, new ArrayList<>());
    }

    /**
     * Construct a new compositeType from XML Schema.
     *
     * @param node           from the XML Schema parsing.
     * @param givenName      for this node.
     * @param referencedName of the type when created from a ref in a composite.
     * @param compositesPath with the path of composites that represents the levels of composition.
     * @throws XPathExpressionException if the XPath is invalid.
     */
    CompositeType(
        final Node node, final String givenName, final String referencedName, final List<String> compositesPath)
        throws XPathExpressionException
    {
        super(node, givenName, referencedName);

        this.compositesPath.addAll(compositesPath);
        this.compositesPath.add(getAttributeValue(node, "name"));

        final XPath xPath = XPathFactory.newInstance().newXPath();
        final NodeList list = (NodeList)xPath.compile(SUB_TYPES_EXP).evaluate(node, NODESET);

        for (int i = 0, size = list.getLength(); i < size; i++)
        {
            final Node subTypeNode = list.item(i);
            final String subTypeName = XmlSchemaParser.getAttributeValue(subTypeNode, "name");

            processType(subTypeNode, subTypeName, null, null);
        }

        checkForValidOffsets(node);
    }

    /**
     * Return the EncodedDataType within this composite with the given name.
     *
     * @param name of the type to return.
     * @return type requested.
     */
    public Type getType(final String name)
    {
        return containedTypeByNameMap.get(name);
    }

    /**
     * The encodedLength (in octets) of the list of encoded types.
     *
     * @return encodedLength of the compositeType.
     */
    public int encodedLength()
    {
        int length = 0;

        for (final Type t : containedTypeByNameMap.values())
        {
            if (t.isVariableLength())
            {
                return Token.VARIABLE_LENGTH;
            }

            if (t.offsetAttribute() != -1)
            {
                length = t.offsetAttribute();
            }

            if (t.presence() != Presence.CONSTANT)
            {
                length += t.encodedLength();
            }
        }

        return length;
    }

    /**
     * Return list of the {@link Type}s that compose this composite.
     *
     * @return {@link List} that holds the {@link Type}s in this composite.
     */
    public List<Type> getTypeList()
    {
        return new ArrayList<>(containedTypeByNameMap.values());
    }

    /**
     * Make this composite type, if it has a varData member, variable length
     * by making the type with the name "varData" be variable length.
     */
    public void makeDataFieldCompositeType()
    {
        final EncodedDataType edt = (EncodedDataType)containedTypeByNameMap.get("varData");
        if (edt != null)
        {
            edt.variableLength(true);
        }
    }

    /**
     * Check the composite for being a well-formed group encodedLength encoding. This means
     * that there are the fields "blockLength" and "numInGroup" present.
     *
     * @param node of the XML for this composite.
     */
    public void checkForWellFormedGroupSizeEncoding(final Node node)
    {
        final EncodedDataType blockLengthType = (EncodedDataType)containedTypeByNameMap.get("blockLength");
        final EncodedDataType numInGroupType = (EncodedDataType)containedTypeByNameMap.get("numInGroup");

        if (blockLengthType == null)
        {
            XmlSchemaParser.handleError(node, "composite for group encodedLength encoding must have \"blockLength\"");
        }
        else if (!isUnsigned(blockLengthType.primitiveType()))
        {
            XmlSchemaParser.handleError(node, "\"blockLength\" must be unsigned type");
        }
        else
        {
            if (blockLengthType.primitiveType() != UINT8 && blockLengthType.primitiveType() != UINT16)
            {
                XmlSchemaParser.handleWarning(node, "\"blockLength\" should be UINT8 or UINT16");
            }

            final PrimitiveValue blockLengthTypeMaxValue = blockLengthType.maxValue();
            validateGroupMaxValue(node, blockLengthType.primitiveType(), blockLengthTypeMaxValue);
        }

        if (numInGroupType == null)
        {
            XmlSchemaParser.handleError(node, "composite for group encodedLength encoding must have \"numInGroup\"");
        }
        else if (!isUnsigned(numInGroupType.primitiveType()))
        {
            XmlSchemaParser.handleWarning(node, "\"numInGroup\" should be unsigned type");
            final PrimitiveValue numInGroupMinValue = numInGroupType.minValue();
            if (null == numInGroupMinValue)
            {
                XmlSchemaParser.handleError(node, "\"numInGroup\" minValue must be set for signed types");
            }
            else if (numInGroupMinValue.longValue() < 0)
            {
                XmlSchemaParser.handleError(node,
                    "\"numInGroup\" minValue=" + numInGroupMinValue + " must be greater than zero " +
                    "for signed \"numInGroup\" types");
            }
        }
        else
        {
            if (numInGroupType.primitiveType() != UINT8 && numInGroupType.primitiveType() != UINT16)
            {
                XmlSchemaParser.handleWarning(node, "\"numInGroup\" should be UINT8 or UINT16");
            }

            final PrimitiveValue numInGroupMaxValue = numInGroupType.maxValue();
            validateGroupMaxValue(node, numInGroupType.primitiveType(), numInGroupMaxValue);

            final PrimitiveValue numInGroupMinValue = numInGroupType.minValue();
            if (null != numInGroupMinValue)
            {
                final long max = numInGroupMaxValue != null ?
                    numInGroupMaxValue.longValue() : numInGroupType.primitiveType().maxValue().longValue();

                if (numInGroupMinValue.longValue() > max)
                {
                    XmlSchemaParser.handleError(
                        node, "\"numInGroup\" minValue=" + numInGroupMinValue + " greater than maxValue=" + max);
                }
            }
        }
    }

    /**
     * Check the composite for being a well-formed variable length data encoding. This means
     * that there are the fields "length" and "varData" present.
     *
     * @param node of the XML for this composite
     */
    public void checkForWellFormedVariableLengthDataEncoding(final Node node)
    {
        final EncodedDataType lengthType = (EncodedDataType)containedTypeByNameMap.get("length");

        if (lengthType == null)
        {
            XmlSchemaParser.handleError(node, "composite for variable length data encoding must have \"length\"");
        }
        else
        {
            final PrimitiveType primitiveType = lengthType.primitiveType();
            if (!isUnsigned(primitiveType))
            {
                XmlSchemaParser.handleError(node, "\"length\" must be unsigned type");
            }
            else if (primitiveType != UINT8 && primitiveType != UINT16 && primitiveType != UINT32)
            {
                XmlSchemaParser.handleWarning(node, "\"length\" should be UINT8, UINT16, or UINT32");
            }

            validateGroupMaxValue(node, primitiveType, lengthType.maxValue());
        }

        if ("optional".equals(getAttributeValueOrNull(node, "presence")))
        {
            XmlSchemaParser.handleError(
                node, "composite for variable length data encoding cannot have presence=\"optional\"");
        }

        if (containedTypeByNameMap.get("varData") == null)
        {
            XmlSchemaParser.handleError(node, "composite for variable length data encoding must have \"varData\"");
        }
    }

    private static void validateGroupMaxValue(
        final Node node, final PrimitiveType primitiveType, final PrimitiveValue value)
    {
        if (null != value)
        {
            final long longValue = value.longValue();
            final long allowedValue = primitiveType.maxValue().longValue();
            if (longValue > allowedValue)
            {
                XmlSchemaParser.handleError(
                    node, "maxValue greater than allowed for type: maxValue=" + longValue + " allowed=" + allowedValue);
            }

            final long maxInt = INT32.maxValue().longValue();
            if (primitiveType == UINT32 && longValue > maxInt)
            {
                XmlSchemaParser.handleError(
                    node, "maxValue greater than allowed for type: maxValue=" + longValue + " allowed=" + maxInt);
            }
        }
        else if (primitiveType == UINT32)
        {
            final long maxInt = INT32.maxValue().longValue();
            XmlSchemaParser.handleError(
                node, "maxValue must be set for varData UINT32 type: max value allowed=" + maxInt);
        }
    }

    /**
     * Check the composite for being a well-formed message headerStructure encoding. This means
     * that there are the fields "blockLength", "templateId" and "version" present.
     *
     * @param node of the XML for this composite
     */
    public void checkForWellFormedMessageHeader(final Node node)
    {
        final boolean shouldGenerateInterfaces = "true".equals(System.getProperty(JAVA_GENERATE_INTERFACES));

        final EncodedDataType blockLengthType = (EncodedDataType)containedTypeByNameMap.get("blockLength");
        final EncodedDataType templateIdType = (EncodedDataType)containedTypeByNameMap.get("templateId");
        final EncodedDataType schemaIdType = (EncodedDataType)containedTypeByNameMap.get("schemaId");
        final EncodedDataType versionType = (EncodedDataType)containedTypeByNameMap.get("version");

        if (blockLengthType == null)
        {
            XmlSchemaParser.handleError(node, "composite for message header must have \"blockLength\"");
        }
        else if (!isUnsigned(blockLengthType.primitiveType()))
        {
            XmlSchemaParser.handleError(node, "\"blockLength\" must be unsigned");
        }

        validateHeaderField(node, "blockLength", blockLengthType, UINT16, shouldGenerateInterfaces);
        validateHeaderField(node, "templateId", templateIdType, UINT16, shouldGenerateInterfaces);
        validateHeaderField(node, "schemaId", schemaIdType, UINT16, shouldGenerateInterfaces);
        validateHeaderField(node, "version", versionType, UINT16, shouldGenerateInterfaces);
    }

    private void validateHeaderField(
        final Node node,
        final String fieldName,
        final EncodedDataType actualType,
        final PrimitiveType expectedType,
        final boolean shouldGenerateInterfaces)
    {
        if (actualType == null)
        {
            XmlSchemaParser.handleError(node, "composite for message header must have \"" + fieldName + "\"");
        }
        else if (actualType.primitiveType() != expectedType)
        {
            XmlSchemaParser.handleWarning(node, "\"" + fieldName + "\" should be " + expectedType.name());

            if (shouldGenerateInterfaces)
            {
                if (actualType.primitiveType().size() > expectedType.size())
                {
                    final String msg = "\"" + fieldName + "\" must be less than " + expectedType.size() +
                        " bytes to use " + JAVA_GENERATE_INTERFACES;
                    XmlSchemaParser.handleError(node, msg);
                }
                else
                {
                    final String msg = "\"" + fieldName + "\" will be cast to " + expectedType.name() +
                        " to use " + JAVA_GENERATE_INTERFACES;
                    XmlSchemaParser.handleWarning(node, msg);
                }
            }
        }
    }

    /**
     * Check the composite for any specified offsets and validate they are correctly specified.
     *
     * @param node of the XML for this composite
     */
    public void checkForValidOffsets(final Node node)
    {
        int offset = 0;

        for (final Type edt : containedTypeByNameMap.values())
        {
            final int offsetAttribute = edt.offsetAttribute();

            if (-1 != offsetAttribute)
            {
                if (offsetAttribute < offset)
                {
                    XmlSchemaParser.handleError(
                        node, "composite element \"" + edt.name() + "\" has incorrect offset specified");
                }

                offset = offsetAttribute;
            }

            offset += edt.encodedLength();
        }
    }

    /**
     * {@inheritDoc}
     */
    public boolean isVariableLength()
    {
        return false;
    }

    private Type processType(
        final Node subTypeNode, final String subTypeName, final String givenName, final String referencedName)
        throws XPathExpressionException
    {
        final String nodeName = subTypeNode.getNodeName();
        Type type = null;

        switch (nodeName)
        {
            case "type":
                type = addType(subTypeNode, subTypeName, new EncodedDataType(subTypeNode, givenName, referencedName));
                break;

            case "enum":
                type = addType(subTypeNode, subTypeName, new EnumType(subTypeNode, givenName, referencedName));
                break;

            case "set":
                type = addType(subTypeNode, subTypeName, new SetType(subTypeNode, givenName, referencedName));
                break;

            case "composite":
                type = addType(
                    subTypeNode,
                    subTypeName,
                    new CompositeType(subTypeNode, givenName, referencedName, compositesPath));
                break;

            case "ref":
            {
                final XPath xPath = XPathFactory.newInstance().newXPath();
                final String refTypeName = XmlSchemaParser.getAttributeValue(subTypeNode, "type");
                final String expression = "/*[local-name() = 'messageSchema']/types/*[@name='" + refTypeName + "']";
                final Node refTypeNode = (Node)xPath.compile(expression)
                    .evaluate(subTypeNode.getOwnerDocument(), XPathConstants.NODE);

                if (refTypeNode == null)
                {
                    XmlSchemaParser.handleError(subTypeNode, "ref type not found: " + refTypeName);
                }
                else
                {
                    if (compositesPath.contains(refTypeName))
                    {
                        XmlSchemaParser.handleError(refTypeNode, "ref types cannot create circular dependencies.");
                        throw new IllegalStateException("ref types cannot create circular dependencies");
                    }

                    final String refName = XmlSchemaParser.getAttributeValue(subTypeNode, "name");
                    type = processType(refTypeNode, refName, refName, refTypeName);

                    final String refOffset = XmlSchemaParser.getAttributeValueOrNull(subTypeNode, "offset");
                    if (null != refOffset)
                    {
                        try
                        {
                            type.offsetAttribute(Integer.parseInt(refOffset));
                        }
                        catch (final NumberFormatException ex)
                        {
                            XmlSchemaParser.handleError(subTypeNode, "invalid number type: " + refOffset);
                        }
                    }

                    final String refVersion = XmlSchemaParser.getAttributeValueOrNull(subTypeNode, "sinceVersion");
                    if (null != refVersion)
                    {
                        try
                        {
                            type.sinceVersion(Integer.parseInt(refVersion));
                        }
                        catch (final NumberFormatException ex)
                        {
                            XmlSchemaParser.handleError(subTypeNode, "invalid number type: " + refVersion);
                        }
                    }
                }

                break;
            }

            case "data":
            case "group":
                XmlSchemaParser.handleError(subTypeNode, nodeName + " not valid within composite");
                break;

            default:
                throw new IllegalStateException("Unknown node type: name=" + nodeName);
        }

        return type;
    }

    private Type addType(final Node subTypeNode, final String name, final Type type)
    {
        if (containedTypeByNameMap.put(name, type) != null)
        {
            XmlSchemaParser.handleError(subTypeNode, "composite already contains a type named: " + name);
        }

        return type;
    }

    /**
     * {@inheritDoc}
     */
    public String toString()
    {
        return "CompositeType{" +
            "compositesPath=" + compositesPath +
            ", containedTypeByNameMap=" + containedTypeByNameMap +
            '}';
    }
}
