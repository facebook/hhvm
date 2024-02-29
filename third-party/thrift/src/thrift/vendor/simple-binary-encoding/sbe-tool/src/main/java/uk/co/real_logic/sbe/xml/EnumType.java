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

import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;
import java.util.Collection;
import java.util.LinkedHashMap;
import java.util.Map;

import static uk.co.real_logic.sbe.xml.Presence.OPTIONAL;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.*;

/**
 * SBE enum type for representing an enumeration of values.
 */
public class EnumType extends Type
{
    /**
     * SBE schema enum type.
     */
    public static final String ENUM_TYPE = "enum";

    private final PrimitiveType encodingType;
    private final PrimitiveValue nullValue;
    private final Map<PrimitiveValue, ValidValue> validValueByPrimitiveValueMap = new LinkedHashMap<>();
    private final Map<String, ValidValue> validValueByNameMap = new LinkedHashMap<>();

    EnumType(final Node node) throws XPathExpressionException
    {
        this(node, null, null);
    }

    /**
     * Construct a new enumType from XML Schema.
     *
     * @param node           from the XML Schema Parsing
     * @param givenName      for the node.
     * @param referencedName of the type when created from a ref in a composite.
     * @throws XPathExpressionException if the XPath is invalid
     */
    EnumType(final Node node, final String givenName, final String referencedName)
        throws XPathExpressionException
    {
        super(node, givenName, referencedName);

        final XPath xPath = XPathFactory.newInstance().newXPath();
        final String encodingTypeStr = getAttributeValue(node, "encodingType");
        final EncodedDataType encodedDataType;

        switch (encodingTypeStr)
        {
            case "char":
            case "uint8":
            case "int8":
            case "int16":
            case "uint16":
            case "int32":
                encodingType = PrimitiveType.get(encodingTypeStr);
                encodedDataType = null;
                break;

            default:
                // might not have run into this type yet, so look for it
                final String expression = TYPE_XPATH_EXPR + "[@name='" + encodingTypeStr + "']";
                final Node encodingTypeNode = (Node)xPath.compile(expression)
                    .evaluate(node.getOwnerDocument(), XPathConstants.NODE);

                if (null == encodingTypeNode)
                {
                    throw new IllegalArgumentException("illegal encodingType for enum " + encodingTypeStr);
                }

                encodedDataType = new EncodedDataType(encodingTypeNode);

                if (encodedDataType.length() != 1)
                {
                    throw new IllegalArgumentException(
                        "illegal encodingType for enum " + encodingTypeStr + " length not equal to 1");
                }

                encodingType = encodedDataType.primitiveType();
        }

        final String nullValueStr = getAttributeValueOrNull(node, "nullValue");
        if (null != nullValueStr)
        {
            nullValue = PrimitiveValue.parse(nullValueStr, encodingType);
        }
        else if (null != encodedDataType && null != encodedDataType.nullValue())
        {
            nullValue = encodedDataType.nullValue();
        }
        else
        {
            nullValue = encodingType.nullValue();
        }

        if (presence() == OPTIONAL && null == nullValue)
        {
            handleError(node, "presence optional but no null value found");
        }

        final NodeList list = (NodeList)xPath.compile("validValue").evaluate(node, XPathConstants.NODESET);

        for (int i = 0, size = list.getLength(); i < size; i++)
        {
            final ValidValue v = new ValidValue(list.item(i), encodingType);

            if (validValueByPrimitiveValueMap.get(v.primitiveValue()) != null)
            {
                handleWarning(node, "validValue already exists for value: " + v.primitiveValue());
            }

            if (validValueByNameMap.get(v.name()) != null)
            {
                handleWarning(node, "validValue already exists for name: " + v.name());
            }

            if (PrimitiveType.CHAR != encodingType)
            {
                final long value = v.primitiveValue().longValue();
                final long minValue = null != encodedDataType && null != encodedDataType.minValue() ?
                    encodedDataType.minValue().longValue() : encodingType.minValue().longValue();
                final long maxValue = null != encodedDataType && null != encodedDataType.maxValue() ?
                    encodedDataType.maxValue().longValue() : encodingType.maxValue().longValue();
                final long nullLongValue = nullValue.longValue();

                if (nullLongValue == value)
                {
                    handleError(node, "validValue " + v.name() + " uses nullValue: " + nullLongValue);
                }
                else if (value < minValue || value > maxValue)
                {
                    handleError(
                        node,
                        "validValue " + v.name() + " outside of range " + minValue + " - " + maxValue + ": " + value);
                }
            }

            validValueByPrimitiveValueMap.put(v.primitiveValue(), v);
            validValueByNameMap.put(v.name(), v);
        }
    }

    /**
     * The {@link PrimitiveType} used to encode the enum.
     *
     * @return the {@link PrimitiveType} used to encode the enum.
     */
    public PrimitiveType encodingType()
    {
        return encodingType;
    }

    /**
     * The encodedLength (in octets) of the encodingType
     *
     * @return encodedLength of the encodingType
     */
    public int encodedLength()
    {
        if (presence() == Presence.CONSTANT)
        {
            return 0;
        }

        return encodingType.size();
    }

    /**
     * Get the {@link ValidValue} represented by a {@link PrimitiveValue}.
     *
     * @param value to lookup
     * @return the {@link ValidValue} represented by a {@link PrimitiveValue} or null.
     */
    public ValidValue getValidValue(final PrimitiveValue value)
    {
        return validValueByPrimitiveValueMap.get(value);
    }

    /**
     * Get the {@link ValidValue} represented by a string name.
     *
     * @param name to lookup
     * @return the {@link ValidValue} represented by a string name or null.
     */
    public ValidValue getValidValue(final String name)
    {
        return validValueByNameMap.get(name);
    }

    /**
     * The nullValue of the type.
     *
     * @return value of the nullValue.
     */
    public PrimitiveValue nullValue()
    {
        return nullValue;
    }

    /**
     * The collection of valid values for this enumeration.
     *
     * @return the collection of valid values for this enumeration.
     */
    public Collection<ValidValue> validValues()
    {
        return validValueByNameMap.values();
    }

    /**
     * {@inheritDoc}
     */
    public boolean isVariableLength()
    {
        return false;
    }

    /**
     * {@inheritDoc}
     */
    public String toString()
    {
        return "EnumType{" +
            "encodingType=" + encodingType +
            ", nullValue=" + nullValue +
            ", validValueByPrimitiveValueMap=" + validValueByPrimitiveValueMap +
            ", validValueByNameMap=" + validValueByNameMap +
            '}';
    }

    /**
     * Holder for valid values for and {@link EnumType}.
     */
    public static class ValidValue
    {
        private final String name;
        private final String description;
        private final PrimitiveValue value;
        private final int sinceVersion;
        private final int deprecated;

        /**
         * Construct a ValidValue given the XML node and the encodingType.
         *
         * @param node         that contains the validValue
         * @param encodingType for the enum
         */
        public ValidValue(final Node node, final PrimitiveType encodingType)
        {
            name = getAttributeValue(node, "name");
            description = getAttributeValueOrNull(node, "description");
            value = PrimitiveValue.parse(node.getFirstChild().getNodeValue(), encodingType);
            sinceVersion = Integer.parseInt(getAttributeValue(node, "sinceVersion", "0"));
            deprecated = Integer.parseInt(getAttributeValue(node, "deprecated", "0"));

            checkForValidName(node, name);
        }

        /**
         * {@link PrimitiveType} for the {@link ValidValue}.
         *
         * @return {@link PrimitiveType} for the {@link ValidValue}.
         */
        public PrimitiveValue primitiveValue()
        {
            return value;
        }

        /**
         * The name of the {@link ValidValue}.
         *
         * @return the name of the {@link ValidValue}
         */
        public String name()
        {
            return name;
        }

        /**
         * The description of the {@link ValidValue}.
         *
         * @return the description of the {@link ValidValue}.
         */
        public String description()
        {
            return description;
        }

        /**
         * The sinceVersion value of the {@link ValidValue}
         *
         * @return the sinceVersion value of the {@link ValidValue}
         */
        public int sinceVersion()
        {
            return sinceVersion;
        }

        /**
         * Version in which {@link ValidValue} was deprecated. Only valid if greater than zero.
         *
         * @return version in which the {@link ValidValue} was deprecated.
         */
        public int deprecated()
        {
            return deprecated;
        }

        /**
         * {@inheritDoc}
         */
        public String toString()
        {
            return "ValidValue{" +
                "name='" + name + '\'' +
                ", description='" + description + '\'' +
                ", value=" + value +
                ", sinceVersion=" + sinceVersion +
                ", deprecated=" + deprecated +
                '}';
        }
    }
}
