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

import org.junit.jupiter.api.Test;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import uk.co.real_logic.sbe.PrimitiveType;
import uk.co.real_logic.sbe.PrimitiveValue;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.core.Is.is;
import static org.junit.jupiter.api.Assertions.*;

class CompositeTypeTest
{
    @Test
    void shouldHandleDecimalCompositeType() throws Exception
    {
        final String testXmlString =
            "<types>" +
            "<composite name=\"decimal\">" +
            "    <type name=\"mantissa\" primitiveType=\"int64\"/>" +
            "    <type name=\"exponent\" primitiveType=\"int8\"/>" +
            "</composite>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/composite", testXmlString);
        final CompositeType decimal = (CompositeType)map.get("decimal");

        assertThat(decimal.name(), is("decimal"));

        final EncodedDataType mantissa = (EncodedDataType)decimal.getType("mantissa");
        final EncodedDataType exponent = (EncodedDataType)decimal.getType("exponent");

        assertThat(mantissa.primitiveType(), is(PrimitiveType.INT64));
        assertThat(exponent.primitiveType(), is(PrimitiveType.INT8));
        assertThat(decimal.encodedLength(), is(9));
    }

    @Test
    void shouldHandleDecimal32CompositeType() throws Exception
    {
        final String testXmlString =
            "<types>" +
            "<composite name=\"decimal32\">" +
            "    <type name=\"mantissa\" primitiveType=\"int32\"/>" +
            "    <type name=\"exponent\" primitiveType=\"int8\" presence=\"constant\">-2</type>" +
            "</composite>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/composite", testXmlString);
        final CompositeType decimal32 = (CompositeType)map.get("decimal32");

        assertThat(decimal32.name(), is("decimal32"));

        final EncodedDataType mantissa = (EncodedDataType)decimal32.getType("mantissa");
        final EncodedDataType exponent = (EncodedDataType)decimal32.getType("exponent");

        assertThat(mantissa.primitiveType(), is(PrimitiveType.INT32));
        assertThat(exponent.primitiveType(), is(PrimitiveType.INT8));
        assertThat(exponent.presence(), is(Presence.CONSTANT));
        assertThat(exponent.constVal(), is(PrimitiveValue.parse("-2", PrimitiveType.INT8)));
        assertThat(decimal32.encodedLength(), is(4));
    }

    @Test
    void shouldHandleDecimal64CompositeType() throws Exception
    {
        final String testXmlString =
            "<types>" +
            "<composite name=\"decimal64\">" +
            "    <type name=\"mantissa\" primitiveType=\"int64\"/>" +
            "    <type name=\"exponent\" primitiveType=\"int8\" presence=\"constant\">-2</type>" +
            "</composite>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/composite", testXmlString);
        final CompositeType decimal64 = (CompositeType)map.get("decimal64");

        assertThat(decimal64.name(), is("decimal64"));
        final EncodedDataType mantissa = (EncodedDataType)decimal64.getType("mantissa");
        final EncodedDataType exponent = (EncodedDataType)decimal64.getType("exponent");

        assertThat(mantissa.primitiveType(), is(PrimitiveType.INT64));
        assertThat(exponent.primitiveType(), is(PrimitiveType.INT8));
        assertThat(exponent.presence(), is(Presence.CONSTANT));
        assertThat(exponent.constVal(), is(PrimitiveValue.parse("-2", PrimitiveType.INT8)));
        assertThat(decimal64.encodedLength(), is(8));
    }

    @Test
    void shouldHandleCompositeTypeList() throws Exception
    {
        final String testXmlString =
            "<types>" +
            "<composite name=\"decimal\">" +
            "    <type name=\"mantissa\" primitiveType=\"int64\"/>" +
            "    <type name=\"exponent\" primitiveType=\"int8\"/>" +
            "</composite>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/composite", testXmlString);
        final CompositeType c = (CompositeType)map.get("decimal");

        assertThat(c.getTypeList().size(), is(2));
        assertThat(c.getTypeList().get(0).name(), is("mantissa"));
        assertThat(c.getTypeList().get(1).name(), is("exponent"));
    }

    @Test
    void shouldHandleCompositeHasNullableType() throws Exception
    {
        final String nullValStr = "9223372036854775807";
        final String testXmlString =
            "<types>" +
            "<composite name=\"PRICENULL\" description=\"Price NULL\" semanticType=\"Price\">" +
            "    <type name=\"mantissa\" description=\"mantissa\" presence=\"optional\" nullValue=\"" +
            nullValStr + "\" primitiveType=\"int64\"/>" +
            "    <type name=\"exponent\" description=\"exponent\" presence=\"constant\" primitiveType=\"int8\">" +
            "-7</type>" +
            "</composite>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/composite", testXmlString);
        final CompositeType c = (CompositeType)map.get("PRICENULL");
        final EncodedDataType mantissa = (EncodedDataType)c.getType("mantissa");

        assertThat(mantissa.nullValue(), is(PrimitiveValue.parse(nullValStr, PrimitiveType.INT64)));
    }

    @Test
    void shouldThrowExceptionWhenCompositeTypeHasTypeNameDuplicates()
    {
        final String testXmlString =
            "<types>" +
            "<composite name=\"decimal\">" +
            "    <type name=\"mantissa\" primitiveType=\"int64\"/>" +
            "    <type name=\"mantissa\" primitiveType=\"int64\"/>" +
            "    <type name=\"exponent\" primitiveType=\"int8\"/>" +
            "</composite>" +
            "</types>";

        assertThrows(IllegalArgumentException.class,
            () -> parseTestXmlWithMap("/types/composite", testXmlString));
    }

    private static Map<String, Type> parseTestXmlWithMap(final String xPathExpr, final String xml)
        throws ParserConfigurationException, XPathExpressionException, IOException, SAXException
    {
        final Document document = DocumentBuilderFactory.newInstance().newDocumentBuilder().parse(
            new ByteArrayInputStream(xml.getBytes()));
        final XPath xPath = XPathFactory.newInstance().newXPath();
        final NodeList list = (NodeList)xPath.compile(xPathExpr).evaluate(document, XPathConstants.NODESET);
        final Map<String, Type> map = new HashMap<>();

        final ParserOptions options = ParserOptions.builder().stopOnError(true).suppressOutput(true).build();
        document.setUserData(XmlSchemaParser.ERROR_HANDLER_KEY, new ErrorHandler(options), null);

        for (int i = 0, size = list.getLength(); i < size; i++)
        {
            final Type t = new CompositeType(list.item(i));
            map.put(t.name(), t);
        }

        return map;
    }
}
