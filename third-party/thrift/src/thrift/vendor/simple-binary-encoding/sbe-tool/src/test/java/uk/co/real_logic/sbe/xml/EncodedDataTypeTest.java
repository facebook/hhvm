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
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static uk.co.real_logic.sbe.PrimitiveValue.parse;

class EncodedDataTypeTest
{
    @Test
    void shouldHandleSettingAllAttributes() throws Exception
    {
        final String testXmlString =
            "<types>" +
            "    <type name=\"testType\" presence=\"required\" primitiveType=\"char\" length=\"1\" " +
            "variableLength=\"false\"/>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/type", testXmlString);
        // assert that testType is in map and name of Type is correct
        final Type t = map.get("testType");

        assertThat(t.name(), is("testType"));
        assertThat(t.presence(), is(Presence.REQUIRED));

        final EncodedDataType d = (EncodedDataType)t;

        assertThat(d.primitiveType(), is(PrimitiveType.CHAR));
        assertThat(d.length(), is(1));
        assertThat(d.isVariableLength(), is(false));
    }

    @Test
    void shouldHandleMultipleTypes() throws Exception
    {
        final String testXmlString =
            "<types>" +
            "    <type name=\"testType1\" presence=\"required\" primitiveType=\"char\" length=\"1\" " +
            "variableLength=\"false\"/>" +
            "    <type name=\"testType2\" presence=\"required\" primitiveType=\"int8\" length=\"1\" " +
            "variableLength=\"false\"/>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/type", testXmlString);

        // assert that testType is in map and name of Type is correct
        assertThat(map.size(), is(2));
        assertThat(map.get("testType1").name(), is("testType1"));
        assertThat(map.get("testType2").name(), is("testType2"));
    }

    @Test
    void shouldSetAppropriateDefaultsWhenNoneSpecified() throws Exception
    {
        final String testXmlString =
            "<types>" +
            "    <type name=\"testType\" primitiveType=\"char\"/>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/type", testXmlString);
        // assert that testType is in map and name of Type is correct
        assertThat(map.get("testType").name(), is("testType"));
        // assert defaults for length, variableLength and presence
        final Type t = map.get("testType");
        assertThat(t.presence(), is(Presence.REQUIRED));

        final EncodedDataType d = (EncodedDataType)t;
        assertThat(d.length(), is(1));
        assertThat(d.isVariableLength(), is(false));
    }

    @Test
    void shouldUseAppropriatePresence() throws Exception
    {
        final String testXmlString =
            "<types>" +
            "    <type name=\"testTypeDefault\" primitiveType=\"char\"/>" +
            "    <type name=\"testTypeRequired\" presence=\"required\" primitiveType=\"char\"/>" +
            "    <type name=\"testTypeOptional\" presence=\"optional\" primitiveType=\"char\"/>" +
            "    <type name=\"testTypeConstant\" presence=\"constant\" primitiveType=\"char\">A</type>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/type", testXmlString);

        assertThat(map.get("testTypeDefault").presence(), is(Presence.REQUIRED));
        assertThat(map.get("testTypeRequired").presence(), is(Presence.REQUIRED));
        assertThat(map.get("testTypeOptional").presence(), is(Presence.OPTIONAL));
        assertThat(map.get("testTypeConstant").presence(), is(Presence.CONSTANT));
    }

    @Test
    void shouldThrowExceptionWhenUnknownPresenceSpecified()
    {
        final String testXmlString =
            "<types>" +
            "    <type name=\"testTyeUnknown\" presence=\"XXXXX\" primitiveType=\"char\"/>" +
            "</types>";

        assertThrows(IllegalArgumentException.class, () -> parseTestXmlWithMap("/types/type", testXmlString));
    }

    @Test
    void shouldThrowExceptionWhenNoPrimitiveTypeSpecified()
    {
        final String testXmlString =
            "<types>" +
            "    <type name=\"testType\"/>" +
            "</types>";

        assertThrows(IllegalStateException.class, () -> parseTestXmlWithMap("/types/type", testXmlString));
    }

    @Test
    void shouldThrowExceptionWhenNoNameSpecified()
    {
        final String testXmlString =
            "<types>" +
            "    <type primitiveType=\"char\"/>" +
            "</types>";

        assertThrows(IllegalStateException.class, () -> parseTestXmlWithMap("/types/type", testXmlString));
    }

    @Test
    void shouldUseAppropriatePrimitiveType() throws Exception
    {
        final String testXmlString =
            "<types>" +
            "    <type name=\"testTypeChar\" primitiveType=\"char\"/>" +
            "    <type name=\"testTypeInt8\" primitiveType=\"int8\"/>" +
            "    <type name=\"testTypeInt16\" primitiveType=\"int16\"/>" +
            "    <type name=\"testTypeInt32\" primitiveType=\"int32\"/>" +
            "    <type name=\"testTypeInt64\" primitiveType=\"int64\"/>" +
            "    <type name=\"testTypeUInt8\" primitiveType=\"uint8\"/>" +
            "    <type name=\"testTypeUInt16\" primitiveType=\"uint16\"/>" +
            "    <type name=\"testTypeUInt32\" primitiveType=\"uint32\"/>" +
            "    <type name=\"testTypeUInt64\" primitiveType=\"uint64\"/>" +
            "    <type name=\"testTypeFloat\" primitiveType=\"float\"/>" +
            "    <type name=\"testTypeDouble\" primitiveType=\"double\"/>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/type", testXmlString);

        assertThat(((EncodedDataType)map.get("testTypeChar")).primitiveType(), is(PrimitiveType.CHAR));
        assertThat(((EncodedDataType)map.get("testTypeInt8")).primitiveType(), is(PrimitiveType.INT8));
        assertThat(((EncodedDataType)map.get("testTypeInt16")).primitiveType(), is(PrimitiveType.INT16));
        assertThat(((EncodedDataType)map.get("testTypeInt32")).primitiveType(), is(PrimitiveType.INT32));
        assertThat(((EncodedDataType)map.get("testTypeInt64")).primitiveType(), is(PrimitiveType.INT64));
        assertThat(((EncodedDataType)map.get("testTypeUInt8")).primitiveType(), is(PrimitiveType.UINT8));
        assertThat(((EncodedDataType)map.get("testTypeUInt16")).primitiveType(), is(PrimitiveType.UINT16));
        assertThat(((EncodedDataType)map.get("testTypeUInt32")).primitiveType(), is(PrimitiveType.UINT32));
        assertThat(((EncodedDataType)map.get("testTypeUInt64")).primitiveType(), is(PrimitiveType.UINT64));
        assertThat(((EncodedDataType)map.get("testTypeFloat")).primitiveType(), is(PrimitiveType.FLOAT));
        assertThat(((EncodedDataType)map.get("testTypeDouble")).primitiveType(), is(PrimitiveType.DOUBLE));
    }

    @Test
    void shouldThrowExceptionWhenUnknownPrimitiveTypeSpecified()
    {
        final String testXmlString =
            "<types>" +
            "    <type name=\"testTypeUnknown\" primitiveType=\"XXXX\"/>" +
            "</types>";

        assertThrows(IllegalArgumentException.class,
            () -> parseTestXmlWithMap("/types/type", testXmlString));
    }

    @Test
    void shouldReturnCorrectSizeForPrimitiveTypes() throws Exception
    {
        final String testXmlString =
            "<types>" +
            "    <type name=\"testTypeChar\" primitiveType=\"char\"/>" +
            "    <type name=\"testTypeInt8\" primitiveType=\"int8\"/>" +
            "    <type name=\"testTypeInt16\" primitiveType=\"int16\"/>" +
            "    <type name=\"testTypeInt32\" primitiveType=\"int32\"/>" +
            "    <type name=\"testTypeInt64\" primitiveType=\"int64\"/>" +
            "    <type name=\"testTypeUInt8\" primitiveType=\"uint8\"/>" +
            "    <type name=\"testTypeUInt16\" primitiveType=\"uint16\"/>" +
            "    <type name=\"testTypeUInt32\" primitiveType=\"uint32\"/>" +
            "    <type name=\"testTypeUInt64\" primitiveType=\"uint64\"/>" +
            "    <type name=\"testTypeFloat\" primitiveType=\"float\"/>" +
            "    <type name=\"testTypeDouble\" primitiveType=\"double\"/>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/type", testXmlString);

        assertThat(map.get("testTypeChar").encodedLength(), is(1));
        assertThat(map.get("testTypeInt8").encodedLength(), is(1));
        assertThat(map.get("testTypeInt32").encodedLength(), is(4));
        assertThat(map.get("testTypeInt64").encodedLength(), is(8));
        assertThat(map.get("testTypeUInt8").encodedLength(), is(1));
        assertThat(map.get("testTypeUInt16").encodedLength(), is(2));
        assertThat(map.get("testTypeUInt32").encodedLength(), is(4));
        assertThat(map.get("testTypeUInt64").encodedLength(), is(8));
        assertThat(map.get("testTypeFloat").encodedLength(), is(4));
        assertThat(map.get("testTypeDouble").encodedLength(), is(8));
    }

    @Test
    void shouldReturnCorrectDescriptionForType() throws Exception
    {
        final String desc = "basic description attribute of a type element";
        final String testXmlString =
            "<types>" +
            "    <type name=\"testTypeDescription\" primitiveType=\"char\" description=\"" + desc + "\"/>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/type", testXmlString);

        assertThat(map.get("testTypeDescription").description(), is(desc));
    }

    @Test
    void shouldReturnNullOnNoDescriptionSet() throws Exception
    {
        final String testXmlString =
            "<types>" +
            "    <type name=\"testTypeNoDescription\" primitiveType=\"char\"/>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/type", testXmlString);
        final String description = map.get("testTypeNoDescription").description();

        assertNull(description);
    }

    @Test
    void shouldReturnCorrectSemanticTypeForType() throws Exception
    {
        final String semanticType = "char";
        final String testXmlString =
            "<types>" +
            "    <type name=\"testType\" primitiveType=\"char\" semanticType=\"" + semanticType + "\"/>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/type", testXmlString);

        assertThat(map.get("testType").semanticType(), is(semanticType));
    }

    @Test
    void shouldReturnNullWhenSemanticTypeNotSpecified() throws Exception
    {
        final String testXmlString =
            "<types>" +
            "    <type name=\"testType\" primitiveType=\"char\"/>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/type", testXmlString);

        assertNull(map.get("testType").semanticType());
    }

    @Test
    void shouldThrowExceptionWhenConstantPresenceButNoDataSpecified()
    {
        final String testXmlString =
            "<types>" +
            "    <type name=\"testTypePresenceConst\" primitiveType=\"char\" presence=\"constant\"></type>" +
            "</types>";

        assertThrows(IllegalArgumentException.class, () -> parseTestXmlWithMap("/types/type", testXmlString));
    }

    @Test
    void shouldReturnCorrectPresenceConstantWhenSpecified() throws Exception
    {
        final String testXmlString =
            "<types>" +
            "    <type name=\"testTypePresenceConst\" primitiveType=\"char\" presence=\"constant\">F</type>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/type", testXmlString);
        final String expectedString = "F";
        final PrimitiveValue expectedValue = parse(expectedString, PrimitiveType.CHAR);

        assertThat((((EncodedDataType)map.get("testTypePresenceConst")).constVal()), is(expectedValue));
    }

    @Test
    void shouldReturnCorrectConstantStringWhenSpecified() throws Exception
    {
        final String strConst = "string constant";
        final String testXmlString =
            "<types>" +
            "    <type name=\"testTypeConstString\" primitiveType=\"char\" presence=\"constant\" " +
            "length=\"" + strConst.length() + "\"" +
            ">" + strConst + "</type>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/type", testXmlString);

        assertThat((((EncodedDataType)map.get("testTypeConstString")).constVal()),
            is(parse(strConst, strConst.length(), "US-ASCII")));
    }

    @Test
    void shouldReturnDefaultMinValueWhenSpecified() throws Exception
    {
        final String testXmlString =
            "<types>" +
            "    <type name=\"testTypeDefaultCharMinValue\" primitiveType=\"char\"/>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/type", testXmlString);

        assertNull(((EncodedDataType)map.get("testTypeDefaultCharMinValue")).minValue());
    }

    @Test
    void shouldReturnDefaultMaxValueWhenSpecified() throws Exception
    {
        final String testXmlString =
            "<types>" +
            "    <type name=\"testTypeDefaultCharMaxValue\" primitiveType=\"char\"/>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/type", testXmlString);

        assertNull(((EncodedDataType)map.get("testTypeDefaultCharMaxValue")).maxValue());
    }

    @Test
    void shouldReturnDefaultNullValueWhenSpecified() throws Exception
    {
        final String testXmlString =
            "<types>" +
            "    <type name=\"testTypeDefaultCharNullValue\" primitiveType=\"char\"/>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/type", testXmlString);

        assertNull(((EncodedDataType)map.get("testTypeDefaultCharNullValue")).nullValue());
    }

    @Test
    void shouldReturnCorrectMinValueWhenSpecified() throws Exception
    {
        final String minVal = "10";
        final String testXmlString =
            "<types>" +
            "    <type name=\"testTypeInt8MinValue\" primitiveType=\"int8\" minValue=\"" + minVal + "\"/>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/type", testXmlString);
        assertThat((((EncodedDataType)map.get("testTypeInt8MinValue")).minValue()),
            is(parse(minVal, PrimitiveType.INT8)));
    }

    @Test
    void shouldReturnCorrectMaxValueWhenSpecified() throws Exception
    {
        final String maxVal = "10";
        final String testXmlString =
            "<types>" +
            "    <type name=\"testTypeInt8MaxValue\" primitiveType=\"int8\" maxValue=\"" + maxVal + "\"/>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/type", testXmlString);

        assertThat((((EncodedDataType)map.get("testTypeInt8MaxValue")).maxValue()),
            is(parse(maxVal, PrimitiveType.INT8)));
    }

    @Test
    void shouldReturnCorrectNullValueWhenSpecified() throws Exception
    {
        final String nullVal = "10";
        final String testXmlString =
            "<types>" +
            "    <type name=\"testTypeInt8NullValue\" primitiveType=\"int8\" presence=\"optional\" nullValue=\"" +
            nullVal + "\"/>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/type", testXmlString);

        assertThat((((EncodedDataType)map.get("testTypeInt8NullValue")).nullValue()),
            is(parse(nullVal, PrimitiveType.INT8)));
    }

    @Test
    void shouldReturnCharacterEncodingWhenSpecified() throws Exception
    {
        final String testXmlString =
            "<types>" +
            "    <type name=\"testTypeCharacterEncoding\" primitiveType=\"char\" length=\"3\" " +
            "characterEncoding=\"cp912\"/>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/type", testXmlString);

        assertThat((((EncodedDataType)map.get("testTypeCharacterEncoding")).characterEncoding()), is("cp912"));
    }

    @Test
    void shouldReturnCharacterEncodingWhenSpecifiedNonCharType() throws Exception
    {
        final String testXmlString =
            "<types>" +
            "    <type name=\"testTypeCharacterEncodingNonChar\" primitiveType=\"uint8\" " +
            "characterEncoding=\"  windows-1251\n\r\"/>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/type", testXmlString);

        assertThat(
            (((EncodedDataType)map.get("testTypeCharacterEncodingNonChar")).characterEncoding()), is("windows-1251"));
    }

    @Test
    void shouldReturnUsAsciiWhenCharacterEncodingNotSpecifiedForTypeChar() throws Exception
    {
        final String testXmlString =
            "<types>" +
            "    <type name=\"testCharDefaultCharacterEncoding\" primitiveType=\"char\" length=\"5\"/>" +
            "</types>";

        final Map<String, Type> map = parseTestXmlWithMap("/types/type", testXmlString);

        assertThat(
            (((EncodedDataType)map.get("testCharDefaultCharacterEncoding")).characterEncoding()), is("US-ASCII"));
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
            final Type t = new EncodedDataType(list.item(i));
            map.put(t.name(), t);
        }

        return map;
    }
}
