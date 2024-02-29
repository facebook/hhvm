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

import uk.co.real_logic.sbe.ir.Token;
import uk.co.real_logic.sbe.PrimitiveType;
import uk.co.real_logic.sbe.PrimitiveValue;

import org.w3c.dom.Node;

import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathException;
import javax.xml.xpath.XPathFactory;

import static uk.co.real_logic.sbe.xml.Presence.CONSTANT;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.handleError;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.handleWarning;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.getAttributeValue;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.getAttributeValueOrNull;

/**
 * SBE simple encoded data type.
 */
public class EncodedDataType extends Type
{
    /**
     * SBE schema type.
     */
    public static final String ENCODED_DATA_TYPE = "type";

    private final PrimitiveType primitiveType;
    private final int length;
    private final PrimitiveValue constValue;
    private final PrimitiveValue minValue;
    private final PrimitiveValue maxValue;
    private final PrimitiveValue nullValue;
    private final String characterEncoding;
    private final String valueRef;
    private boolean varLen;

    /**
     * Construct a new encodedDataType from XML Schema.
     *
     * @param node from the XML Schema Parsing
     */
    public EncodedDataType(final Node node)
    {
        this(node, null, null);
    }

    /**
     * Construct a new encodedDataType from XML Schema.
     *
     * @param node           from the XML Schema Parsing.
     * @param givenName      for this node.
     * @param referencedName of the type when created from a ref in a composite.
     */
    @SuppressWarnings("this-escape")
    public EncodedDataType(final Node node, final String givenName, final String referencedName)
    {
        super(node, givenName, referencedName);

        primitiveType = PrimitiveType.get(getAttributeValue(node, "primitiveType"));
        final String lengthAttr = getAttributeValueOrNull(node, "length");
        length = Integer.parseInt(null == lengthAttr ? "1" : lengthAttr);
        varLen = Boolean.parseBoolean(getAttributeValue(node, "variableLength", "false"));
        valueRef = getAttributeValueOrNull(node, "valueRef");

        if (null != valueRef)
        {
            if (valueRef.indexOf('.') == -1)
            {
                handleError(node, "valueRef format not valid (enum-name.valid-value-name): " + valueRef);
            }

            if (presence() != CONSTANT)
            {
                handleError(node, "present must be constant when valueRef is set: " + valueRef);
            }
        }

        if (PrimitiveType.CHAR == primitiveType)
        {
            characterEncoding = getAttributeValue(node, "characterEncoding", "US-ASCII").trim();
        }
        else
        {
            final String configuredCharacterEncoding = getAttributeValueOrNull(node, "characterEncoding");
            characterEncoding = configuredCharacterEncoding == null ? null : configuredCharacterEncoding.trim();
        }

        if (presence() == CONSTANT)
        {
            if (null == valueRef)
            {
                if (node.getFirstChild() == null)
                {
                    handleError(node, "type has declared presence as \"constant\" but XML node has no data");
                    constValue = null;
                }
                else
                {
                    final String nodeValue = node.getFirstChild().getNodeValue();
                    if (PrimitiveType.CHAR == primitiveType)
                    {
                        constValue = processConstantChar(node, lengthAttr, nodeValue);
                    }
                    else
                    {
                        constValue = PrimitiveValue.parse(nodeValue, primitiveType);
                    }
                }
            }
            else
            {
                constValue = lookupValueRef(node);
            }
        }
        else
        {
            constValue = null;
        }

        final String minValStr = getAttributeValueOrNull(node, "minValue");
        minValue = minValStr != null ? PrimitiveValue.parse(minValStr, primitiveType) : null;

        final String maxValStr = getAttributeValueOrNull(node, "maxValue");
        maxValue = maxValStr != null ? PrimitiveValue.parse(maxValStr, primitiveType) : null;

        final String nullValStr = getAttributeValueOrNull(node, "nullValue");
        if (nullValStr != null)
        {
            if (presence() != Presence.OPTIONAL)
            {
                handleWarning(node, "nullValue set, but presence is not optional");
            }

            nullValue = PrimitiveValue.parse(nullValStr, primitiveType);
        }
        else
        {
            nullValue = null;
        }
    }

    private PrimitiveValue lookupValueRef(final Node node)
    {
        try
        {
            final int periodIndex = valueRef.indexOf('.');
            final String valueRefType = valueRef.substring(0, periodIndex);

            final XPath xPath = XPathFactory.newInstance().newXPath();
            final Node valueRefNode = (Node)xPath.compile(
                "/*[local-name() = 'messageSchema']/types/enum[@name='" + valueRefType + "']")
                .evaluate(node.getOwnerDocument(), XPathConstants.NODE);

            if (valueRefNode == null)
            {
                XmlSchemaParser.handleError(node, "valueRef not found: " + valueRefType);
                return null;
            }

            final EnumType enumType = new EnumType(valueRefNode);
            if (enumType.encodingType() != primitiveType)
            {
                handleError(node, "valueRef does not match this type: " + valueRef);
                return null;
            }

            final String validValueName = valueRef.substring(periodIndex + 1);
            final EnumType.ValidValue validValue = enumType.getValidValue(validValueName);

            if (null == validValue)
            {
                handleError(node, "valueRef for validValue name not found: " + validValueName);
                return null;
            }

            return validValue.primitiveValue();
        }
        catch (final XPathException ex)
        {
            throw new RuntimeException(ex);
        }
    }

    /**
     * Construct a new EncodedDataType with direct values. Does not handle constant values.
     *
     * @param name          of the type.
     * @param presence      of the type.
     * @param description   of the type or null.
     * @param semanticType  of the type or null.
     * @param primitiveType of the EncodedDataType.
     * @param length        of the EncodedDataType.
     * @param varLen        of the EncodedDataType.
     */
    public EncodedDataType(
        final String name,
        final Presence presence,
        final String description,
        final String semanticType,
        final PrimitiveType primitiveType,
        final int length,
        final boolean varLen)
    {
        this(name, null, presence, description, semanticType, primitiveType, length, varLen);
    }

    /**
     * Construct a new EncodedDataType with direct values. Does not handle constant values.
     *
     * @param name          of the type.
     * @param packageName   of the type.
     * @param presence      of the type.
     * @param description   of the type or null.
     * @param semanticType  of the type or null.
     * @param primitiveType of the EncodedDataType.
     * @param length        of the EncodedDataType.
     * @param varLen        of the EncodedDataType.
     */
    public EncodedDataType(
        final String name,
        final String packageName,
        final Presence presence,
        final String description,
        final String semanticType,
        final PrimitiveType primitiveType,
        final int length,
        final boolean varLen)
    {
        super(name, packageName, presence, description, 0, 0, semanticType);

        this.primitiveType = primitiveType;
        this.length = length;
        this.varLen = varLen;
        this.constValue = null;
        this.minValue = null;
        this.maxValue = null;
        this.nullValue = null;
        characterEncoding = null;
        valueRef = null;
    }

    /**
     * Return the length attribute of the type
     *
     * @return length attribute of the type
     */
    public int length()
    {
        return length;
    }

    /**
     * Return the variableLength attribute of the type
     *
     * @return variableLength boolean of the type
     */
    public boolean isVariableLength()
    {
        return varLen;
    }

    /**
     * Set if the type is variable length or not.
     *
     * @param variableLength true if variable length.
     */
    public void variableLength(final boolean variableLength)
    {
        this.varLen = variableLength;
    }

    /**
     * Return the primitiveType attribute of the type
     *
     * @return primitiveType attribute of the type
     */
    public PrimitiveType primitiveType()
    {
        return primitiveType;
    }

    /**
     * The encodedLength (in octets) of the encoding as length of the primitiveType times its count.
     *
     * @return encodedLength of the encoding
     */
    public int encodedLength()
    {
        if (presence() == CONSTANT)
        {
            return 0;
        }

        if (varLen)
        {
            return Token.VARIABLE_LENGTH;
        }

        return primitiveType.size() * length;
    }

    /**
     * The constant value of the type if specified
     *
     * @return value of the constant for this type
     */
    public PrimitiveValue constVal()
    {
        if (presence() != CONSTANT)
        {
            throw new IllegalStateException("type is not of constant presence");
        }

        return constValue;
    }

    /**
     * The minValue of the type
     *
     * @return value of the minValue
     */
    public PrimitiveValue minValue()
    {
        return minValue;
    }

    /**
     * The maxValue of the type
     *
     * @return value of the maxValue
     */
    public PrimitiveValue maxValue()
    {
        return maxValue;
    }

    /**
     * The nullValue of the type
     *
     * @return value of the nullValue primitiveType or type
     */
    public PrimitiveValue nullValue()
    {
        return nullValue;
    }

    /**
     * The character encoding of the type
     *
     * @return value representing the encoding
     */
    public String characterEncoding()
    {
        return characterEncoding;
    }

    /**
     * Get the value of the valueRef attribute.
     *
     * @return the value of the valueRef attribute.
     */
    public String valueRef()
    {
        return valueRef;
    }

    private PrimitiveValue processConstantChar(final Node node, final String lengthAttr, final String nodeValue)
    {
        final int valueLength = nodeValue.length();

        if (null != lengthAttr && length < valueLength)
        {
            handleError(node, "length of " + length + " is less than provided value: " + nodeValue);
        }

        final PrimitiveValue primitiveValue;

        if (valueLength == 1)
        {
            if (null == lengthAttr || length == 1)
            {
                primitiveValue = PrimitiveValue.parse(nodeValue, primitiveType, characterEncoding);
            }
            else
            {
                primitiveValue = PrimitiveValue.parse(nodeValue, length, characterEncoding);
            }
        }
        else
        {
            if (null == lengthAttr)
            {
                primitiveValue = PrimitiveValue.parse(nodeValue, valueLength, characterEncoding);
            }
            else
            {
                primitiveValue = PrimitiveValue.parse(nodeValue, length, characterEncoding);
            }
        }

        return primitiveValue;
    }

    /**
     * {@inheritDoc}
     */
    public String toString()
    {
        return "EncodedDataType{" +
            "primitiveType=" + primitiveType +
            ", length=" + length +
            ", constValue=" + constValue +
            ", minValue=" + minValue +
            ", maxValue=" + maxValue +
            ", nullValue=" + nullValue +
            ", characterEncoding='" + characterEncoding + '\'' +
            ", valueRef='" + valueRef + '\'' +
            ", varLen=" + varLen +
            '}';
    }
}
