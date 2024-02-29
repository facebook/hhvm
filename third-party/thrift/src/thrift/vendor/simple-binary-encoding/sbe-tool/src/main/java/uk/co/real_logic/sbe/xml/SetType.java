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

import static uk.co.real_logic.sbe.xml.XmlSchemaParser.*;

/**
 * SBE set tpe representing a bitset of options.
 */
public class SetType extends Type
{
    /**
     * SBE schema set type.
     */
    public static final String SET_TYPE = "set";

    private final PrimitiveType encodingType;
    private final Map<PrimitiveValue, Choice> choiceByPrimitiveValueMap = new LinkedHashMap<>();
    private final Map<String, Choice> choiceByNameMap = new LinkedHashMap<>();

    SetType(final Node node)
        throws XPathExpressionException
    {
        this(node, null, null);
    }

    /**
     * Construct a new SetType from XML Schema.
     *
     * @param node           from the XML Schema Parsing
     * @param givenName      for the node.
     * @param referencedName of the type when created from a ref in a composite.
     * @throws XPathExpressionException on invalid XPath.
     */
    public SetType(final Node node, final String givenName, final String referencedName)
        throws XPathExpressionException
    {
        super(node, givenName, referencedName);

        final XPath xPath = XPathFactory.newInstance().newXPath();
        final String encodingTypeStr = getAttributeValue(node, "encodingType");

        switch (encodingTypeStr)
        {
            case "uint8":
            case "uint16":
            case "uint32":
            case "uint64":
                encodingType = PrimitiveType.get(encodingTypeStr);
                break;

            default:
                // might not have run into this type yet, so look for it
                final String expression = TYPE_XPATH_EXPR + "[@name='" + encodingTypeStr + "']";
                final Node encodingTypeNode = (Node)xPath
                    .compile(expression)
                    .evaluate(node.getOwnerDocument(), XPathConstants.NODE);

                if (null == encodingTypeNode)
                {
                    encodingType = null;
                }
                else if (Integer.parseInt(getAttributeValue(encodingTypeNode, "length", "1")) != 1)
                {
                    encodingType = null;
                }
                else
                {
                    encodingType = PrimitiveType.get(getAttributeValue(encodingTypeNode, "primitiveType"));
                }
                break;
        }

        if (encodingType == null)
        {
            throw new IllegalArgumentException("Illegal encodingType " + encodingTypeStr);
        }

        final NodeList list = (NodeList)xPath.compile("choice").evaluate(node, XPathConstants.NODESET);

        for (int i = 0, size = list.getLength(); i < size; i++)
        {
            final Choice c = new Choice(list.item(i), encodingType);

            if (choiceByPrimitiveValueMap.get(c.primitiveValue()) != null)
            {
                handleWarning(node, "Choice value already defined: " + c.primitiveValue());
            }

            if (choiceByNameMap.get(c.name()) != null)
            {
                handleWarning(node, "Choice already exists for name: " + c.name());
            }

            choiceByPrimitiveValueMap.put(c.primitiveValue(), c);
            choiceByNameMap.put(c.name(), c);
        }
    }

    /**
     * The encoding type of the bitset to be used on the wire.
     *
     * @return encoding type of the bitset to be used on the wire.
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
        return encodingType.size();
    }

    /**
     * Get the {@link Choice} represented by a {@link PrimitiveValue}.
     *
     * @param value to get
     * @return the {@link Choice} represented by a {@link PrimitiveValue} or null if not found.
     */
    public Choice getChoice(final PrimitiveValue value)
    {
        return choiceByPrimitiveValueMap.get(value);
    }

    /**
     * Get the {@link Choice} represented by a String name.
     *
     * @param name to get
     * @return the {@link Choice} represented by a String name or null if not found.
     */
    public Choice getChoice(final String name)
    {
        return choiceByNameMap.get(name);
    }

    /**
     * The collection of possible {@link Choice} values for a bitset.
     *
     * @return the collection of possible {@link Choice} values for a bitset.
     */
    public Collection<Choice> choices()
    {
        return choiceByNameMap.values();
    }

    /**
     * Always false.
     * <p>
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
        return "SetType{" +
            "encodingType=" + encodingType +
            ", choiceByPrimitiveValueMap=" + choiceByPrimitiveValueMap +
            ", choiceByNameMap=" + choiceByNameMap +
            '}';
    }

    /**
     * Holder for valid values for SBE schema enum type.
     */
    public static class Choice
    {
        private final String name;
        private final String description;
        private final PrimitiveValue value;
        private final int sinceVersion;
        private final int deprecated;

        /**
         * Construct a Choice given the XML node and the encodingType
         *
         * @param node         that contains the validValue
         * @param encodingType for the enum
         */
        public Choice(final Node node, final PrimitiveType encodingType)
        {
            name = getAttributeValue(node, "name");
            description = getAttributeValueOrNull(node, "description");
            value = PrimitiveValue.parse(node.getFirstChild().getNodeValue(), encodingType);
            sinceVersion = Integer.parseInt(getAttributeValue(node, "sinceVersion", "0"));
            deprecated = Integer.parseInt(getAttributeValue(node, "deprecated", "0"));

            // choice values are bit positions (0, 1, 2, 3, 4, etc.) from LSB to MSB
            if (value.longValue() >= (encodingType.size() * 8L))
            {
                throw new IllegalArgumentException("Choice value out of bounds: " + value.longValue());
            }

            checkForValidName(node, name);
        }

        /**
         * The {@link PrimitiveValue} representation of the bitset choice.
         *
         * @return the {@link PrimitiveValue} representation of the bitset choice.
         */
        public PrimitiveValue primitiveValue()
        {
            return value;
        }

        /**
         * The String name representation of the bitset choice.
         *
         * @return the String name representation of the bitset choice.
         */
        public String name()
        {
            return name;
        }

        /**
         * The description of the bitset choice.
         *
         * @return the description of the bitset choice.
         */
        public String description()
        {
            return description;
        }

        /**
         * The sinceVersion value of the {@link Choice}
         *
         * @return the sinceVersion value of the {@link Choice}
         */
        public int sinceVersion()
        {
            return sinceVersion;
        }

        /**
         * Version in which {@link Choice} was deprecated. Only valid if greater than zero.
         *
         * @return version in which the {@link Choice} was deprecated.
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
            return "Choice{" +
                "name='" + name + '\'' +
                ", description='" + description + '\'' +
                ", value=" + value +
                ", sinceVersion=" + sinceVersion +
                ", deprecated=" + deprecated +
                '}';
        }
    }
}
