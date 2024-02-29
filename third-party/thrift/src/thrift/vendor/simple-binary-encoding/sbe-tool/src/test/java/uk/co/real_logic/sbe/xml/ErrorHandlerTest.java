/*
 * Copyright 2013-2024 Real Logic Limited.
 * Copyright 2017 MarketFactory Inc.
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

import uk.co.real_logic.sbe.Tests;

import org.junit.jupiter.api.Test;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.core.Is.is;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.fail;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.parse;

class ErrorHandlerTest
{
    @Test
    void shouldNotExitOnTypeErrorsAndWarnings() throws Exception
    {
        final String testXmlString =
            "<types>" +
            "<enum name=\"NullBoolean\" encodingType=\"uint8\" nullValue=\"255\" semanticType=\"Boolean\">" +
            "    <validValue name=\"false\">0</validValue>" +
            "    <validValue name=\"true\">1</validValue>" +
            "</enum>" +
            "<enum name=\"DupNameBoolean\" encodingType=\"uint8\" semanticType=\"Boolean\">" +
            "    <validValue name=\"false\">0</validValue>" +
            "    <validValue name=\"anotherFalse\">0</validValue>" +
            "    <validValue name=\"true\">1</validValue>" +
            "</enum>" +
            "<enum name=\"DupValBoolean\" encodingType=\"uint8\" semanticType=\"Boolean\">" +
            "    <validValue name=\"false\">0</validValue>" +
            "    <validValue name=\"false\">2</validValue>" +
            "    <validValue name=\"true\">1</validValue>" +
            "</enum>" +
            "<set name=\"DupValueSet\" encodingType=\"uint8\">" +
            "    <choice name=\"Bit0\">0</choice>" +
            "    <choice name=\"AnotherBit0\">0</choice>" +
            "</set>" +
            "<set name=\"DupNameSet\" encodingType=\"uint8\">" +
            "    <choice name=\"Bit0\">0</choice>" +
            "    <choice name=\"Bit0\">1</choice>" +
            "</set>" +
            "<composite name=\"decimal\">" +
            "    <type name=\"mantissa\" primitiveType=\"int64\"/>" +
            "    <type name=\"mantissa\" primitiveType=\"int64\"/>" +
            "    <type name=\"exponent\" primitiveType=\"int8\"/>" +
            "</composite>" +
            "<type name=\"ConstButNoValue\" primitiveType=\"char\" presence=\"constant\"></type>" +
            "<type name=\"NullButNotOptional\" primitiveType=\"int8\" presence=\"required\" nullValue=\"10\"/>" +
            "</types>";

        final Map<String, Type> map = new HashMap<>();
        final ParserOptions options = ParserOptions.builder().suppressOutput(true).build();
        final ErrorHandler handler = new ErrorHandler(options);

        parseTestXmlAddToMap(map, "/types/composite", testXmlString, handler);
        parseTestXmlAddToMap(map, "/types/type", testXmlString, handler);
        parseTestXmlAddToMap(map, "/types/enum", testXmlString, handler);
        parseTestXmlAddToMap(map, "/types/set", testXmlString, handler);

        assertThat(handler.errorCount(), is(2));
        assertThat(handler.warningCount(), is(33));
    }

    @Test
    void shouldExitAfterTypes() throws Exception
    {
        try (InputStream in = Tests.getLocalResource("error-handler-types-schema.xml"))
        {
            final ParserOptions options = ParserOptions.builder().suppressOutput(true).build();
            parse(in, options);
        }
        catch (final IllegalStateException ex)
        {
            assertEquals("had 2 errors", ex.getMessage());
            return;
        }

        fail("expected IllegalStateException");
    }

    @Test
    void shouldExitAfterTypesWhenDupTypesDefined() throws Exception
    {
        try (InputStream in = Tests.getLocalResource("error-handler-types-dup-schema.xml"))
        {
            final ParserOptions options = ParserOptions.builder().suppressOutput(true).warningsFatal(true).build();
            parse(in, options);
        }
        catch (final IllegalStateException ex)
        {
            assertEquals("had 1 warning", ex.getMessage());
            return;
        }

        fail("expected IllegalStateException");
    }

    @Test
    void shouldExitAfterMessageWhenDupMessageIdsDefined() throws Exception
    {
        try (InputStream in = Tests.getLocalResource("error-handler-dup-message-schema.xml"))
        {
            final ParserOptions options = ParserOptions.builder().suppressOutput(true).warningsFatal(true).build();
            parse(in, options);
        }
        catch (final IllegalStateException ex)
        {
            assertEquals("had 1 error", ex.getMessage());
            return;
        }

        fail("expected IllegalStateException");
    }

    @Test
    void shouldExitAfterMessage() throws Exception
    {
        try (InputStream in = Tests.getLocalResource("error-handler-message-schema.xml"))
        {
            final ParserOptions options = ParserOptions.builder().suppressOutput(true).warningsFatal(true).build();
            parse(in, options);
        }
        catch (final IllegalStateException ex)
        {
            assertEquals("had 13 errors", ex.getMessage());
            return;
        }

        fail("expected IllegalStateException");
    }

    @Test
    void shouldExitAfterMessageWhenGroupDimensionsNotComposite() throws Exception
    {
        try (InputStream in = Tests.getLocalResource("error-handler-group-dimensions-schema.xml"))
        {
            final ParserOptions options = ParserOptions.builder().suppressOutput(true).warningsFatal(true).build();
            parse(in, options);
        }
        catch (final IllegalStateException ex)
        {
            assertEquals("had 1 error", ex.getMessage());
            return;
        }

        fail("expected IllegalStateException");
    }

    @Test
    void shouldExitAfterTypesWhenCompositeOffsetsIncorrect() throws Exception
    {
        try (InputStream in = Tests.getLocalResource("error-handler-invalid-composite-offsets-schema.xml"))
        {
            final ParserOptions options = ParserOptions.builder().suppressOutput(true).warningsFatal(true).build();
            parse(in, options);
        }
        catch (final IllegalStateException ex)
        {
            assertEquals("had 2 errors", ex.getMessage());
            return;
        }

        fail("expected IllegalStateException");
    }

    @Test
    void shouldExitAfterTypesWhenCompositeHasInvalidTypes() throws Exception
    {
        try (InputStream in = Tests.getLocalResource("error-handler-invalid-composite.xml"))
        {
            final ParserOptions options = ParserOptions.builder().suppressOutput(true).warningsFatal(true).build();
            parse(in, options);
        }
        catch (final IllegalStateException ex)
        {
            assertEquals("had 2 errors", ex.getMessage());
            return;
        }

        fail("expected IllegalStateException");
    }

    @Test
    void shouldExitInvalidFieldNames() throws Exception
    {
        try (InputStream in = Tests.getLocalResource("error-handler-invalid-name.xml"))
        {
            final ParserOptions options = ParserOptions.builder().suppressOutput(true).warningsFatal(true).build();
            parse(in, options);
        }
        catch (final IllegalStateException ex)
        {
            assertEquals("had 16 warnings", ex.getMessage());
            return;
        }

        fail("expected IllegalStateException");
    }

    private static void parseTestXmlAddToMap(
        final Map<String, Type> map, final String xPathExpr, final String xml, final ErrorHandler handler)
        throws ParserConfigurationException, XPathExpressionException, IOException, SAXException
    {
        final Document document = DocumentBuilderFactory.newInstance().newDocumentBuilder().parse(
            new ByteArrayInputStream(xml.getBytes()));
        final XPath xPath = XPathFactory.newInstance().newXPath();
        final NodeList list = (NodeList)xPath.compile(xPathExpr).evaluate(document, XPathConstants.NODESET);

        document.setUserData(XmlSchemaParser.ERROR_HANDLER_KEY, handler, null);

        for (int i = 0, size = list.getLength(); i < size; i++)
        {
            Type type = null;

            if (xPathExpr.endsWith("enum"))
            {
                type = new EnumType(list.item(i));
            }
            else if (xPathExpr.endsWith("set"))
            {
                type = new SetType(list.item(i));
            }
            else if (xPathExpr.endsWith("type"))
            {
                type = new EncodedDataType(list.item(i));
            }
            else if (xPathExpr.endsWith("composite"))
            {
                type = new CompositeType(list.item(i));
            }

            if (type != null)
            {
                map.put(type.name(), type);
            }
        }
    }
}
