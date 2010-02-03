<?php

include_once 'base.php';

p("#include <libxml/parser.h>");
p("#include <libxml/parserInternals.h>");
p("#include <libxml/relaxng.h>");
p("#include <libxml/tree.h>");
p("#include <libxml/uri.h>");
p("#include <libxml/xmlerror.h>");
p("#include <libxml/xmlschemas.h>");
p("#include <libxml/xmlwriter.h>");
p("#include <libxml/xinclude.h>");
p("#include <libxml/hash.h>");
p("#include <libxml/c14n.h>");
p("#include <libxml/HTMLparser.h>");
p("#include <libxml/HTMLtree.h>");
p("#include <libxml/xpath.h>");
p("#include <libxml/xpathInternals.h>");
p("#include <libxml/xpointer.h>");


///////////////////////////////////////////////////////////////////////////////
// object methods

c('DOMNode', null, array(),
  // Methods
  array(
    m(PublicMethod, '__construct', null,
      array()),
    m(PublicMethod, 'appendChild', Variant,
      array('newnode' => Object)),
    m(PublicMethod, 'cloneNode', Variant,
      array('deep' => array(Boolean, 'false'))),
    m(PublicMethod, 'getLineNo', Int64),
    m(PublicMethod, 'hasAttributes', Boolean),
    m(PublicMethod, 'hasChildNodes', Boolean),
    m(PublicMethod, 'insertBefore',  Variant,
      array('newnode' => Object,
            'refnode' => array(Object, 'null'))),
    m(PublicMethod, 'isDefaultNamespace', Boolean,
      array('namespaceURI' => String)),
    m(PublicMethod, 'isSameNode', Boolean,
      array('node' => Object)),
    m(PublicMethod, 'isSupported',  Boolean,
      array('feature' => String,
            'version' => String)),
    m(PublicMethod, 'lookupNamespaceURI',  Variant,
      array('namespaceURI' => String)),
    m(PublicMethod, 'lookupPrefix', Variant,
      array('prefix' => String)),
    m(PublicMethod, 'normalize', null),
    m(PublicMethod, 'removeChild',  Variant,
      array('node' => Object)),
    m(PublicMethod, 'replaceChild', Variant,
      array('newChildObj' => Object,
            'oldChildObj' => Object)),
    m(PublicMethod, 'C14N', Variant,
      array('exclusive' => array(Boolean, 'false'),
            'with_comments' => array(Boolean, 'false'),
            'xpath' => array(Variant, 'null'),
            'ns_prefixes' => array(Variant, 'null'))),
    m(PublicMethod, 'C14NFile', Variant,
      array('uri' => String,
            'exclusive' => array(Boolean, 'false'),
            'with_comments' => array(Boolean, 'false'),
            'xpath' => array(Variant, 'null'),
            'ns_prefixes' => array(Variant, 'null'))),
    m(PublicMethod, 'getNodePath', Variant)
    ),
  // Constants
  array(),
  // Internal fields
  "\n".
  " public:\n".
  "  xmlNodePtr m_node;"
  );

c('DOMAttr', 'DOMNode', array(),
  // Methods
  array(
    m(PublicMethod, '__construct', null,
      array('name' => Variant,
            'value' => Variant)),
    m(PublicMethod, 'isId', Boolean),
    ),
  // Constants
  array(),
  // Internal fields
  ""
  );

c('DOMCharacterData', 'DOMNode', array(),
  // Methods
  array(
    m(PublicMethod, '__construct', null,
      array('value' => Variant)),
    m(PublicMethod, 'appendData', Boolean,
      array('arg' => String)),
    m(PublicMethod, 'deleteData', Boolean,
      array('offset' => Int64,
            'count' => Int64)),
    m(PublicMethod, 'insertData', Boolean,
      array('offset' => Int64,
            'data' => String)),
    m(PublicMethod, 'replaceData', Boolean,
      array('offset' => Int64,
            'count' => Int64,
            'data' => String)),
    m(PublicMethod, 'substringData', String,
      array('offset' => Int64,
            'count' => Int64))
    ),
  // Constants
  array(),
  // Internal fields
  ""
  );

c('DOMComment', 'DOMCharacterData', array(),
  // Methods
  array(
    m(PublicMethod, '__construct', null,
      array('value' => Variant))
    ),
  // Constants
  array(),
  // Internal fields
  ""
  );

c('DOMText', 'DOMCharacterData', array(),
  // Methods
  array(
    m(PublicMethod, '__construct', null,
      array('value' => Variant)),
    m(PublicMethod, 'isWhitespaceInElementContent', Boolean),
    m(PublicMethod, 'splitText', Variant,
      array('offset' => Int64))
    ),
  // Constants
  array(),
  // Internal fields
  ""
  );

c('DOMCdataSection', 'DOMText', array(),
  // Methods
  array(
    m(PublicMethod, '__construct', null,
      array('value' => Variant))
    ),
  // Constants
  array(),
  // Internal fields
  ""
  );

c('DOMDocument', 'DOMNode', array(),
  // Methods
  array(
    m(PublicMethod, '__construct', null,
      array('version' => String,
            'encoding' => String)),
    m(PublicMethod, 'createAttribute', Variant,
      array('name' => String)),
    m(PublicMethod, 'createAttributeNS', Variant,
      array('namespaceURI' => String,
            'qualifiedName' => String)),
    m(PublicMethod, 'createCDATASection', Variant,
      array('data' => String)),
    m(PublicMethod, 'createComment', Variant,
      array('data' => String)),
    m(PublicMethod, 'createDocumentFragment', Variant),
    m(PublicMethod, 'createElement', Variant,
      array('name' => String,
            'value' => array(String, 'null_string'))),
    m(PublicMethod, 'createElementNS', Variant,
      array('namespaceURI' => String,
            'qualifiedName' => String,
            'value' => array(String, 'null_string'))),
    m(PublicMethod, 'createEntityReference', Variant,
      array('name' => String)),
    m(PublicMethod, 'createProcessingInstruction', Variant,
      array('target' => String,
            'data' => array(String, 'null_string'))),
    m(PublicMethod, 'createTextNode', Variant,
      array('data' => String)),
    m(PublicMethod, 'getElementById', Variant,
      array('elementId' => String)),
    m(PublicMethod, 'getElementsByTagName', Variant,
      array('name' => String)),
    m(PublicMethod, 'getElementsByTagNameNS', Variant,
      array('namespaceURI' => String,
           'localName' => String)),
    m(PublicMethod, 'importNode', Variant,
      array('importedNode' => Object,
            'deep' => array(Boolean, 'false'))),
    m(PublicMethod, 'load', Variant,
      array('filename' => String,
            'options' => array(Int64, '0'))),
    m(PublicMethod, 'loadHTML', Variant,
      array('source' => String)),
    m(PublicMethod, 'loadHTMLFile', Variant,
      array('filename' => String)),
    m(PublicMethod, 'loadXML', Variant,
      array('source' => String,
            'options' => array(Int64, '0'))),
    m(PublicMethod, 'normalizeDocument', null),
    m(PublicMethod, 'registerNodeClass', Boolean,
      array('baseclass' => String,
            'extendedclass' => String)),
    m(PublicMethod, 'relaxNGValidate', Boolean,
      array('filename' => String)),
    m(PublicMethod, 'relaxNGValidateSource', Boolean,
      array('source' => String)),
    m(PublicMethod, 'save', Variant,
      array('file' => String,
            'options' => array(Int64, '0'))),
    m(PublicMethod, 'saveHTML', Variant),
    m(PublicMethod, 'saveHTMLFile', Variant,
      array('file' => String)),
    m(PublicMethod, 'saveXML', Variant,
      array('node' => array(Object, 'null_object'),
            'options' => array(Int64, '0'))),
    m(PublicMethod, 'schemaValidate', Boolean,
      array('filename' => String)),
    m(PublicMethod, 'schemaValidateSource', Boolean,
      array('source' => String)),
    m(PublicMethod, 'validate', Boolean),
    m(PublicMethod, 'xinclude', Variant,
      array('options' => array(Int64, '0')))
    ),
  // Constants
  array(),
  // Internal fields
  ""
  );

c('DOMDocumentFragment', 'DOMNode', array(),
  // Methods
  array(
    m(PublicMethod, '__construct', null),
    m(PublicMethod, 'appendXML', Boolean,
      array('data' => String))
    ),
  // Constants
  array(),
  // Internal fields
  ""
  );

c('DOMDocumentType', 'DOMNode', array(),
  // Methods
  array(
    m(PublicMethod, '__construct', null)
    ),
  // Constants
  array(),
  // Internal fields
  ""
  );

c('DOMElement', 'DOMNode', array(),
  // Methods
  array(
    m(PublicMethod, '__construct', null,
      array('name' => Variant,
            'value' => Variant,
            'namespaceURI' => Variant)),
    m(PublicMethod, 'getAttribute', String,
      array('name' => String)),
    m(PublicMethod, 'getAttributeNode', Variant,
      array('name' => String)),
    m(PublicMethod, 'getAttributeNodeNS', Object,
      array('namespaceURI' => String,
            'localName' => String)),
    m(PublicMethod, 'getAttributeNS', String,
      array('namespaceURI' => String,
            'localName' => String)),
    m(PublicMethod, 'getElementsByTagName', Object,
      array('name' => String)),
    m(PublicMethod, 'getElementsByTagNameNS', Object,
      array('namespaceURI' => String,
            'localName' => String)),
    m(PublicMethod, 'hasAttribute', Boolean,
      array('name' => String)),
    m(PublicMethod, 'hasAttributeNS', Boolean,
      array('namespaceURI' => String,
            'localName' => String)),
    m(PublicMethod, 'removeAttribute', Boolean,
      array('name' => String)),
    m(PublicMethod, 'removeAttributeNode', Variant,
      array('oldAttr' => Object)),
    m(PublicMethod, 'removeAttributeNS', Variant,
      array('namespaceURI' => String,
            'localName' => String)),
    m(PublicMethod, 'setAttribute', Variant,
      array('name' => String,
            'value' => String)),
    m(PublicMethod, 'setAttributeNode', Variant,
      array('newAttr' => Object)),
    m(PublicMethod, 'setAttributeNodeNS', Variant,
      array('newAttr' => Object)),
    m(PublicMethod, 'setAttributeNS', Variant,
      array('namespaceURI' => String,
            'name' => String,
            'value' => String)),
    m(PublicMethod, 'setIdAttribute',  Variant,
      array('name' => String,
            'isId' => Boolean)),
    m(PublicMethod, 'setIdAttributeNode', Variant,
      array('idAttr' => Object,
            'isId' => Boolean)),
    m(PublicMethod, 'setIdAttributeNS', Variant,
      array('namespaceURI' => String,
            'localName' => String,
            'isId' => Boolean))
    ),
  // Constants
  array(),
  // Internal fields
  ""
  );

c('DOMEntity', 'DOMNode', array(),
  // Methods
  array(
    m(PublicMethod, '__construct', null)
    ),
  // Constants
  array(),
  // Internal fields
  ""
  );

c('DOMEntityReference', 'DOMNode', array(),
  // Methods
  array(
    m(PublicMethod, '__construct', null,
      array('name' => Variant))
    ),
  // Constants
  array(),
  // Internal fields
  ""
  );

c('DOMNotation', 'DOMNode', array(),
  // Methods
  array(
    m(PublicMethod, '__construct', null)
    ),
  // Constants
  array(),
  // Internal fields
  ""
  );

c('DOMProcessingInstruction', 'DOMNode', array(),
  // Methods
  array(
    m(PublicMethod, '__construct', null,
      array('name' => Variant,
            'value' => array(Variant, 'null_variant')))
    ),
  // Constants
  array(),
  // Internal fields
  ""
  );

c('DOMNamedNodeMap', null, array(),
  // Methods
  array(
    m(PublicMethod, '__construct', null),
    m(PublicMethod, 'getNamedItem', Variant,
      array('name' => String)),
    m(PublicMethod, 'getNamedItemNS', Variant,
      array('namespaceURI' => String,
            'localName' => String)),
    m(PublicMethod, 'item', Object,
      array('index' => Int64))
    ),
  // Constants
  array(),
  // Internal fields
  ""
  );

c('DOMNodeList', null, array(),
  // Methods
  array(
    m(PublicMethod, '__construct', null),
    m(PublicMethod, 'item', Object,
      array('index' => Int64))
    ),
  // Constants
  array(),
  // Internal fields
  ""
  );

c('DOMException', 'Exception', array(),
  // Methods
  array(
    m(PublicMethod, '__construct', null)
    ),
  // Constants
  array(),
  // Internal fields
  ""
  );

c('DOMImplementation', null, array(),
  // Methods
  array(
    m(PublicMethod, '__construct', null),
    m(PublicMethod, 'createDocument', Variant,
      array('namespaceURI' => array(String, 'null_string'),
            'qualifiedName' => array(String, 'null_string'),
            'doctypeObj' => array(Object, 'null_object'))),
    m(PublicMethod, 'createDocumentType', Variant,
      array('qualifiedName' => array(String, 'null_string'),
            'publicId' => array(String, 'null_string'),
            'systemId' => array(String, 'null_string'))),
    m(PublicMethod, 'hasFeature', Boolean,
      array('feature' => String,
            'version' => String))
    ),
  // Constants
  array(),
  // Internal fields
  ""
  );

c('DOMXPath', null, array(),
  // Methods
  array(
    m(PublicMethod, '__construct', null,
      array('doc' => Variant)),
    m(PublicMethod, 'evaluate',  Variant,
      array('expr' => String,
            'context' => array(Object, 'null_object'))),
    m(PublicMethod, 'query', Variant,
      array('expr' => String,
            'context' => array(Object, 'null_object'))),
    m(PublicMethod, 'registerNamespace', Boolean,
      array('prefix' => String,
            'uri' => String)),
    m(PublicMethod, 'registerPhpFunctions', Variant,
      array('funcs' => array(Variant, 'null')))
    ),
  // Constants
  array(),
  // Internal fields
  "\n".
  " public:\n".
  "  xmlNodePtr m_node;"
  );

