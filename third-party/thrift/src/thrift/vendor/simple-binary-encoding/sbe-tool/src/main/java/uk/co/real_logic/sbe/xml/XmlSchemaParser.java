/*
 * Copyright 2013-2024 Real Logic Limited.
 * Copyright 2017 MarketFactory Inc
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

import org.agrona.Strings;
import org.agrona.collections.ObjectHashSet;
import org.w3c.dom.*;
import org.xml.sax.InputSource;
import uk.co.real_logic.sbe.ValidationUtil;

import javax.xml.XMLConstants;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.validation.SchemaFactory;
import javax.xml.xpath.*;
import java.io.File;
import java.io.InputStream;
import java.nio.ByteOrder;
import java.util.HashMap;
import java.util.Map;

import static uk.co.real_logic.sbe.PrimitiveType.*;
import static uk.co.real_logic.sbe.xml.Presence.REQUIRED;

/**
 * Encapsulate the XML Schema parsing for SBE so that other representations may be
 * used to generate independent representations.
 */
public class XmlSchemaParser
{
    /**
     * Key for storing {@link ErrorHandler} as user data in XML document.
     */
    public static final String ERROR_HANDLER_KEY = "SbeErrorHandler";

    static final String TYPE_XPATH_EXPR =
        "/*[local-name() = 'messageSchema']/types/" + EncodedDataType.ENCODED_DATA_TYPE;

    static final String COMPOSITE_XPATH_EXPR =
        "/*[local-name() = 'messageSchema']/types/" + CompositeType.COMPOSITE_TYPE;

    static final String ENUM_XPATH_EXPR =
        "/*[local-name() = 'messageSchema']/types/" + EnumType.ENUM_TYPE;

    static final String SET_XPATH_EXPR =
        "/*[local-name() = 'messageSchema']/types/" + SetType.SET_TYPE;

    static final String MESSAGE_SCHEMA_XPATH_EXPR =
        "/*[local-name() = 'messageSchema']";

    static final String MESSAGE_XPATH_EXPR =
        "/*[local-name() = 'messageSchema']/*[local-name() = 'message']";

    /**
     * Validate the document against a given schema. Errors will be written to {@link java.lang.System#err}.
     *
     * @param xsdFilename schema to validate against.
     * @param is          source from which schema is read. Ideally it will have the systemId property set to resolve
     *                    relative references.
     * @param options     to be applied during parsing.
     * @throws Exception if an error occurs when parsing the document or schema.
     */
    public static void validate(final String xsdFilename, final InputSource is, final ParserOptions options)
        throws Exception
    {
        final DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        final SchemaFactory schemaFactory = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);

        factory.setSchema(schemaFactory.newSchema(new File(xsdFilename)));
        factory.setNamespaceAware(true);

        if (options.xIncludeAware())
        {
            factory.setXIncludeAware(true);
            factory.setFeature("http://apache.org/xml/features/xinclude/fixup-base-uris", false);
        }

        factory.newDocumentBuilder().parse(is);
    }

    /**
     * Wraps the {@link InputStream} into an {@link InputSource} and delegates to
     * {@link #validate(String, InputSource, ParserOptions)}.
     * <p>
     * <b>Note:</b> this method does not set the {@link InputSource#setSystemId(java.lang.String)} property.
     * However, it is recommended to use the {@link #validate(String, InputSource, ParserOptions)}  method directly.
     *
     * @param xsdFilename schema to validate against.
     * @param in          document to be validated.
     * @param options     to be applied during parsing.
     * @throws Exception if an error occurs when parsing the document or schema.
     */
    public static void validate(final String xsdFilename, final InputStream in, final ParserOptions options)
        throws Exception
    {
        validate(xsdFilename, new InputSource(in), options);
    }

    /**
     * Take an {@link InputSource} and parse it generating map of template ID to Message objects, types, and schema.
     *
     * @param is      source from which schema is read. Ideally it will have the systemId property set to resolve
     *                relative references.
     * @param options to be applied during parsing.
     * @return {@link MessageSchema} encoding for the schema.
     * @throws Exception on parsing error.
     */
    public static MessageSchema parse(final InputSource is, final ParserOptions options) throws Exception
    {
        final DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();

        if (options.xIncludeAware())
        {
            factory.setNamespaceAware(true);
            factory.setXIncludeAware(true);
            factory.setFeature("http://apache.org/xml/features/xinclude/fixup-base-uris", false);
        }

        final Document document = factory.newDocumentBuilder().parse(is);
        final ErrorHandler errorHandler = new ErrorHandler(options);
        document.setUserData(ERROR_HANDLER_KEY, errorHandler, null);

        final XPath xPath = XPathFactory.newInstance().newXPath();
        final Map<String, Type> typeByNameMap = findTypes(document, xPath);
        errorHandler.checkIfShouldExit();

        final Map<Long, Message> messageByIdMap = findMessages(document, xPath, typeByNameMap);
        errorHandler.checkIfShouldExit();

        final Node schemaNode = (Node)xPath.compile(MESSAGE_SCHEMA_XPATH_EXPR).evaluate(document, XPathConstants.NODE);
        if (null == schemaNode)
        {
            throw new IllegalStateException("messageSchema element not found in document, schema is not valid for SBE");
        }

        final MessageSchema messageSchema = new MessageSchema(schemaNode, typeByNameMap, messageByIdMap);

        messageSchema.validate(errorHandler);
        errorHandler.checkIfShouldExit();

        return messageSchema;
    }

    /**
     * Wraps the {@link InputStream} into an {@link InputSource} and delegates to
     * {@link #parse(InputSource, ParserOptions)}.
     * <p>
     * <b>Note:</b> this method does not set the {@link InputSource#setSystemId(java.lang.String)} property.
     * However, it is recommended to use the {@link #parse(InputSource, ParserOptions)} method directly.
     *
     * @param in      stream from which schema is read.
     * @param options to be applied during parsing.
     * @return {@link MessageSchema} encoding for the schema.
     * @throws Exception on parsing error.
     */
    public static MessageSchema parse(final InputStream in, final ParserOptions options) throws Exception
    {
        return parse(new InputSource(in), options);
    }

    /**
     * Scan XML for all types (encodedDataType, compositeType, enumType, and setType) and save in a map.
     *
     * @param document for the XML parsing.
     * @param xPath    for XPath expression reuse.
     * @return {@link java.util.Map} of name {@link java.lang.String} to {@link Type}.
     * @throws XPathExpressionException on parsing error.
     */
    public static Map<String, Type> findTypes(final Document document, final XPath xPath)
        throws XPathExpressionException
    {
        final Map<String, Type> typeByNameMap = new HashMap<>();

        typeByNameMap.put("char", new EncodedDataType("char", REQUIRED, null, null, CHAR, 1, false));
        typeByNameMap.put("int8", new EncodedDataType("int8", REQUIRED, null, null, INT8, 1, false));
        typeByNameMap.put("int16", new EncodedDataType("int16", REQUIRED, null, null, INT16, 1, false));
        typeByNameMap.put("int32", new EncodedDataType("int32", REQUIRED, null, null, INT32, 1, false));
        typeByNameMap.put("int64", new EncodedDataType("int64", REQUIRED, null, null, INT64, 1, false));
        typeByNameMap.put("uint8", new EncodedDataType("uint8", REQUIRED, null, null, UINT8, 1, false));
        typeByNameMap.put("uint16", new EncodedDataType("uint16", REQUIRED, null, null, UINT16, 1, false));
        typeByNameMap.put("uint32", new EncodedDataType("uint32", REQUIRED, null, null, UINT32, 1, false));
        typeByNameMap.put("uint64", new EncodedDataType("uint64", REQUIRED, null, null, UINT64, 1, false));
        typeByNameMap.put("float", new EncodedDataType("float", REQUIRED, null, null, FLOAT, 1, false));
        typeByNameMap.put("double", new EncodedDataType("double", REQUIRED, null, null, DOUBLE, 1, false));

        forEach(
            (NodeList)xPath.compile(TYPE_XPATH_EXPR).evaluate(document, XPathConstants.NODESET),
            (node) -> addTypeWithNameCheck(typeByNameMap, new EncodedDataType(node), node));

        forEach(
            (NodeList)xPath.compile(COMPOSITE_XPATH_EXPR).evaluate(document, XPathConstants.NODESET),
            (node) -> addTypeWithNameCheck(typeByNameMap, new CompositeType(node), node));

        forEach(
            (NodeList)xPath.compile(ENUM_XPATH_EXPR).evaluate(document, XPathConstants.NODESET),
            (node) -> addTypeWithNameCheck(typeByNameMap, new EnumType(node), node));

        forEach(
            (NodeList)xPath.compile(SET_XPATH_EXPR).evaluate(document, XPathConstants.NODESET),
            (node) -> addTypeWithNameCheck(typeByNameMap, new SetType(node), node));

        return typeByNameMap;
    }

    /**
     * Scan XML for all message definitions and save in map.
     *
     * @param document      for the XML parsing.
     * @param xPath         for XPath expression reuse.
     * @param typeByNameMap to use for Type objects.
     * @return {@link java.util.Map} of schemaId to {@link Message}.
     * @throws XPathExpressionException on parsing error.
     */
    public static Map<Long, Message> findMessages(
        final Document document, final XPath xPath, final Map<String, Type> typeByNameMap)
        throws XPathExpressionException
    {
        final Map<Long, Message> messageByIdMap = new HashMap<>();
        final ObjectHashSet<String> distinctNames = new ObjectHashSet<>();

        forEach(
            (NodeList)xPath.compile(MESSAGE_XPATH_EXPR).evaluate(document, XPathConstants.NODESET),
            (node) -> addMessageWithIdCheck(distinctNames, messageByIdMap, new Message(node, typeByNameMap), node));

        if (messageByIdMap.isEmpty())
        {
            handleWarning(document.getDocumentElement(), "no messages found in document");
        }

        return messageByIdMap;
    }

    /**
     * Handle an error condition as consequence of parsing.
     *
     * @param node that is the context of the warning.
     * @param msg  associated with the error.
     */
    public static void handleError(final Node node, final String msg)
    {
        final ErrorHandler handler = (ErrorHandler)node.getOwnerDocument().getUserData(ERROR_HANDLER_KEY);
        if (null == handler)
        {
            throw new IllegalStateException("ERROR: " + formatLocationInfo(node) + msg);
        }
        else
        {
            handler.error(formatLocationInfo(node) + msg);
        }
    }

    /**
     * Handle a warning condition as a consequence of parsing.
     *
     * @param node as the context for the warning.
     * @param msg  associated with the warning.
     */
    public static void handleWarning(final Node node, final String msg)
    {
        final ErrorHandler handler = (ErrorHandler)node.getOwnerDocument().getUserData(ERROR_HANDLER_KEY);
        if (null == handler)
        {
            throw new IllegalStateException("WARNING: " + formatLocationInfo(node) + msg);
        }
        else
        {
            handler.warning(formatLocationInfo(node) + msg);
        }
    }

    /**
     * Helper function that throws an exception when the attribute is not set.
     *
     * @param elementNode that should have the attribute.
     * @param attrName    that is to be looked up.
     * @return value of the attribute.
     * @throws IllegalStateException if the attribute is not present.
     */
    public static String getAttributeValue(final Node elementNode, final String attrName)
    {
        if (null == elementNode)
        {
            throw new IllegalStateException(
                "element node is null when looking for attribute: " + attrName);
        }

        final NamedNodeMap attributes = elementNode.getAttributes();
        final Node attrNode = attributes.getNamedItemNS(null, attrName);
        if (null == attrNode)
        {
            throw new IllegalStateException(
                "element '" + elementNode.getNodeName() + "' has missing attribute: " + attrName);
        }

        final String nodeValue = attrNode.getNodeValue();
        if (Strings.isEmpty(nodeValue))
        {
            throw new IllegalStateException(
                "element '" + elementNode.getNodeName() + "' has empty attribute: " + attrName);
        }

        return nodeValue;
    }

    /**
     * Helper function that uses a default value when value not set.
     *
     * @param elementNode that should have the attribute.
     * @param attrName    that is to be looked up.
     * @param defValue    value to return if not set.
     * @return value of the attribute or defValue.
     */
    public static String getAttributeValue(final Node elementNode, final String attrName, final String defValue)
    {
        if (null == elementNode)
        {
            throw new IllegalStateException(
                "element node is null when looking for attribute: " + attrName);
        }

        final NamedNodeMap attributes = elementNode.getAttributes();
        final Node attrNode = attributes.getNamedItemNS(null, attrName);
        if (null == attrNode)
        {
            return defValue;
        }

        return attrNode.getNodeValue();
    }

    /**
     * To be used with descendant elements of {@code <types>} elements. Returns the package attribute value as
     * defined on the ancestor {@code <types>} element.
     *
     * @param elementNode the node inside the {@code <types>} element.
     * @return the package name, or null if not defined.
     */
    public static String getTypesPackageAttribute(final Node elementNode)
    {
        Node parentNode = elementNode.getParentNode();
        while (null != parentNode)
        {
            if ("types".equals(parentNode.getLocalName()) || "types".equals(parentNode.getNodeName()))
            {
                return getAttributeValue(parentNode, "package", null);
            }

            parentNode = parentNode.getParentNode();
        }

        return null;
    }

    /**
     * Helper function that hides the null return from {@link org.w3c.dom.NamedNodeMap#getNamedItem(String)}.
     *
     * @param elementNode that could be null.
     * @param attrName    that is to be looked up.
     * @return null or value of the attribute.
     */
    public static String getAttributeValueOrNull(final Node elementNode, final String attrName)
    {
        if (null == elementNode)
        {
            return null;
        }

        final NamedNodeMap attributes = elementNode.getAttributes();
        if (null == attributes)
        {
            return null;
        }

        final Node attrNode = attributes.getNamedItemNS(null, attrName);
        if (null == attrNode)
        {
            return null;
        }

        return attrNode.getNodeValue();
    }

    /**
     * Helper function to convert a schema byteOrderName into a {@link ByteOrder}.
     *
     * @param byteOrderName specified as a FIX SBE string.
     * @return ByteOrder representation.
     */
    public static ByteOrder getByteOrder(final String byteOrderName)
    {
        if ("bigEndian".equals(byteOrderName))
        {
            return ByteOrder.BIG_ENDIAN;
        }

        return ByteOrder.LITTLE_ENDIAN;
    }

    /**
     * Check name against validity for C++ and Java naming. Warning if not valid.
     *
     * @param node to have the name checked.
     * @param name of the node to be checked.
     */
    public static void checkForValidName(final Node node, final String name)
    {
        if (!ValidationUtil.isSbeCppName(name))
        {
            handleWarning(node, "name is not valid for C++: " + name);
        }

        if (!ValidationUtil.isSbeJavaName(name))
        {
            handleWarning(node, "name is not valid for Java: " + name);
        }

        if (!ValidationUtil.isSbeGolangName(name))
        {
            handleWarning(node, "name is not valid for Golang: " + name);
        }

        if (!ValidationUtil.isSbeCSharpName(name))
        {
            handleWarning(node, "name is not valid for C#: " + name);
        }
    }

    private static void addTypeWithNameCheck(final Map<String, Type> typeByNameMap, final Type type, final Node node)
    {
        if (typeByNameMap.get(type.name()) != null)
        {
            handleWarning(node, "type already exists for name: " + type.name());
        }

        checkForValidName(node, type.name());

        typeByNameMap.put(type.name(), type);
    }

    private static void addMessageWithIdCheck(
        final ObjectHashSet<String> distinctNames,
        final Map<Long, Message> messageByIdMap,
        final Message message,
        final Node node)
    {
        if (messageByIdMap.get((long)message.id()) != null)
        {
            handleError(node, "message template id already exists: " + message.id());
        }

        if (!distinctNames.add(message.name()))
        {
            handleError(node, "message name already exists: " + message.name());
        }

        checkForValidName(node, message.name());

        messageByIdMap.put((long)message.id(), message);
    }

    private static String formatLocationInfo(final Node node)
    {
        final Node parentNode = node.getParentNode();

        return "at " +
            "<" + parentNode.getNodeName() +
            (getAttributeValueOrNull(parentNode, "name") == null ?
                ">" : (" name=\"" + getAttributeValueOrNull(parentNode, "name") + "\"> ")) +
            "<" + node.getNodeName() +
            (getAttributeValueOrNull(node, "name") == null ?
                ">" : (" name=\"" + getAttributeValueOrNull(node, "name") + "\"> "));
    }

    @FunctionalInterface
    interface NodeFunction
    {
        void execute(Node node) throws XPathExpressionException;
    }

    static void forEach(final NodeList nodeList, final NodeFunction func)
        throws XPathExpressionException
    {
        for (int i = 0, size = nodeList.getLength(); i < size; i++)
        {
            func.execute(nodeList.item(i));
        }
    }
}
