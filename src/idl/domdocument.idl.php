<?php

include_once 'base.php';

p("#include <runtime/ext/ext_domdocument_includes.h>");

///////////////////////////////////////////////////////////////////////////////
// object methods

c('DOMNode', null, array('Sweepable' => 'internal'),
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
    m(PublicMethod, 'getNodePath', Variant),
    m(PublicMethod, "__get", Variant,
      array('name' => Variant)),
    m(PublicMethod, "__set", Variant,
      array('name' => Variant,
            'value' => Variant)),
    ),
  // Constants
  array(),
  // Internal fields
  "\n".
  "public:\n".
  "  sp_domdocument m_doc;\n".
  "  xmlNodePtr m_node;\n".
  "  bool m_owner;"
 );

c('DOMAttr', 'DOMNode', array(),
  array(
    m(PublicMethod, '__construct', null,
      array('name' => String,
            'value' => array(String, 'null_string'))),
    m(PublicMethod, 'isId', Boolean),
    m(PublicMethod, "__get", Variant,
      array('name' => Variant)),
    m(PublicMethod, "__set", Variant,
      array('name' => Variant,
            'value' => Variant)),
  )
 );

c('DOMCharacterData', 'DOMNode', array(),
  array(
    m(PublicMethod, '__construct', null),
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
            'count' => Int64)),
    m(PublicMethod, "__get", Variant,
      array('name' => Variant)),
    m(PublicMethod, "__set", Variant,
      array('name' => Variant,
            'value' => Variant)),
  )
 );

c('DOMComment', 'DOMCharacterData', array(),
  array(
    m(PublicMethod, '__construct', null,
      array('value' => array(String, 'null_string'))),
  )
 );

c('DOMText', 'DOMCharacterData', array(),
  array(
    m(PublicMethod, '__construct', null,
      array('value' => array(String, 'null_string'))),
    m(PublicMethod, 'isWhitespaceInElementContent', Boolean),
    m(PublicMethod, 'splitText', Variant,
      array('offset' => Int64)),
    m(PublicMethod, "__get", Variant,
      array('name' => Variant)),
    m(PublicMethod, "__set", Variant,
      array('name' => Variant,
            'value' => Variant)),
  )
 );

c('DOMCdataSection', 'DOMText', array(),
  array(
    m(PublicMethod, '__construct', null,
      array('value' => String))
  )
 );

c('DOMDocument', 'DOMNode', array(),
  array(
    m(PublicMethod, '__construct', null,
      array('version' => array(String, 'null_string'),
            'encoding' => array(String, 'null_string'))),
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
      array('options' => array(Int64, '0'))),
    m(PublicMethod, "__get", Variant,
      array('name' => Variant)),
    m(PublicMethod, "__set", Variant,
      array('name' => Variant,
            'value' => Variant)),
    ),
  // Constants
  array(),
  // Internal fields
  "\n".
  "public:\n".
  "  bool m_formatoutput;\n".
  "  bool m_validateonparse;\n".
  "  bool m_resolveexternals;\n".
  "  bool m_preservewhitespace;\n".
  "  bool m_substituteentities;\n".
  "  bool m_stricterror;\n".
  "  bool m_recover;\n".
  "  Array m_classmap;"
 );

c('DOMDocumentFragment', 'DOMNode', array(),
  array(
    m(PublicMethod, '__construct', null),
    m(PublicMethod, 'appendXML', Boolean,
      array('data' => String))
  )
 );

c('DOMDocumentType', 'DOMNode', array(),
  array(
    m(PublicMethod, '__construct', null),
    m(PublicMethod, "__get", Variant,
      array('name' => Variant)),
    m(PublicMethod, "__set", Variant,
      array('name' => Variant,
            'value' => Variant)),
  )
 );

c('DOMElement', 'DOMNode', array(),
  array(
    m(PublicMethod, '__construct', null,
      array('name' => String,
            'value' => array(String, 'null_string'),
            'namespaceURI' => array(String, 'null_string'))),
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
            'isId' => Boolean)),
    m(PublicMethod, "__get", Variant,
      array('name' => Variant)),
    m(PublicMethod, "__set", Variant,
      array('name' => Variant,
            'value' => Variant)),
  )
 );

c('DOMEntity', 'DOMNode', array(),
  array(
    m(PublicMethod, '__construct', null),
    m(PublicMethod, "__get", Variant,
      array('name' => Variant)),
    m(PublicMethod, "__set", Variant,
      array('name' => Variant,
            'value' => Variant)),
  )
 );

c('DOMEntityReference', 'DOMNode', array(),
  array(
    m(PublicMethod, '__construct', null,
      array('name' => String))
  )
 );

c('DOMNotation', 'DOMNode', array(),
  array(
    m(PublicMethod, '__construct', null),
    m(PublicMethod, "__get", Variant,
      array('name' => Variant)),
    m(PublicMethod, "__set", Variant,
      array('name' => Variant,
            'value' => Variant)),
  )
 );

c('DOMProcessingInstruction', 'DOMNode', array(),
  array(
    m(PublicMethod, '__construct', null,
      array('name' => String,
            'value' => array(String, 'null_string'))),
    m(PublicMethod, "__get", Variant,
      array('name' => Variant)),
    m(PublicMethod, "__set", Variant,
      array('name' => Variant,
            'value' => Variant)),
  )
 );

c('DOMNodeIterator', null, array('Iterator', 'Sweepable' => 'internal'),
  array(
    m(PublicMethod, '__construct', null),
    m(PublicMethod, 'current', Variant),
    m(PublicMethod, 'key',     Variant),
    m(PublicMethod, 'next',    Variant),
    m(PublicMethod, 'rewind',  Variant),
    m(PublicMethod, 'valid',   Variant),
  ),
  // Constants
  array(),
  // Internal fields
  "\n".
  "public:\n".
  "  void reset_iterator(dom_iterable *objmap);\n".
  "\n".
  "  dom_iterable *m_objmap;\n".
  "  ArrayIter *m_iter;\n".
  "  int m_index;\n".
  "  Object m_curobj;"
 );

c('DOMNamedNodeMap', null, array('IteratorAggregate',
                                 'dom_iterable' => 'internal'),
  array(
    m(PublicMethod, '__construct', null),
    m(PublicMethod, 'getNamedItem', Variant,
      array('name' => String)),
    m(PublicMethod, 'getNamedItemNS', Variant,
      array('namespaceURI' => String,
            'localName' => String)),
    m(PublicMethod, 'item', Variant,
      array('index' => Int64)),
    m(PublicMethod, "__get", Variant,
      array('name' => Variant)),
    m(PublicMethod, "__set", Variant,
      array('name' => Variant,
            'value' => Variant)),
    m(PublicMethod, "getiterator", Variant),
  )
 );

c('DOMNodeList', null, array('IteratorAggregate',
                             'dom_iterable' => 'internal'),
  array(
    m(PublicMethod, '__construct', null),
    m(PublicMethod, 'item', Variant,
      array('index' => Int64)),
    m(PublicMethod, "__get", Variant,
      array('name' => Variant)),
    m(PublicMethod, "__set", Variant,
      array('name' => Variant,
            'value' => Variant)),
    m(PublicMethod, "getiterator", Variant),
  )
 );

c('DOMException', 'Exception', array(),
  array(
    m(PublicMethod, '__construct', null,
      array('message' => array(String, '""'),
            'code' => array(Int64, '0'))),
  )
 );

c('DOMImplementation', null, array(),
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
  )
 );

c('DOMXPath', null, array('Sweepable' => 'internal'),
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
      array('funcs' => array(Variant, 'null'))),
    m(PublicMethod, "__get", Variant,
      array('name' => Variant)),
    m(PublicMethod, "__set", Variant,
      array('name' => Variant,
            'value' => Variant)),
    ),
  // Constants
  array(),
  // Internal fields
  "\n".
  " public:\n".
  "  xmlNodePtr m_node;\n".
  "  sp_domdocument m_doc;\n".
  "  Array m_node_list;\n".
  "  int m_registerPhpFunctions;\n".
  "  Array m_registered_phpfunctions;"
 );

///////////////////////////////////////////////////////////////////////////////
// functions

/* domdocument methods */
f('dom_document_create_element', Variant,
  array('obj' => Variant,
        'name' => String,
        'value' => array(String, 'null_string')));
f('dom_document_create_document_fragment', Variant,
  array('obj' => Variant));
f('dom_document_create_text_node', Variant,
  array('obj' => Variant,
        'data' => String));
f('dom_document_create_comment', Variant,
  array('obj' => Variant,
        'data' => String));
f('dom_document_create_cdatasection', Variant,
  array('obj' => Variant,
        'data' => String));
f('dom_document_create_processing_instruction', Variant,
  array('obj' => Variant,
        'target' => String,
        'data' => array(String, 'null_string')));
f('dom_document_create_attribute', Variant,
  array('obj' => Variant,
        'name' => String));
f('dom_document_create_entity_reference', Variant,
  array('obj' => Variant,
        'name' => String));
f('dom_document_get_elements_by_tag_name', Variant,
  array('obj' => Variant,
        'name' => String));
f('dom_document_import_node', Variant,
  array('obj' => Variant,
        'importedNode' => Object,
        'deep' => array(Boolean, 'false')));
f('dom_document_create_element_ns', Variant,
  array('obj' => Variant,
        'namespaceURI' => String,
        'qualifiedName' => String,
        'value' => array(String, 'null_string')));
f('dom_document_create_attribute_ns', Variant,
  array('obj' => Variant,
        'namespaceURI' => String,
        'qualifiedName' => String));
f('dom_document_get_elements_by_tag_name_ns', Variant,
  array('obj' => Variant,
        'namespaceURI' => String,
        'localName' => String));
f('dom_document_get_element_by_id', Variant,
  array('obj' => Variant,
        'elementId' => String));
f('dom_document_normalize_document', Variant,
  array('obj' => Variant));
f('dom_document_save', Variant,
  array('obj' => Variant,
        'file' => String,
        'options' => array(Int64, '0')));
f('dom_document_savexml', Variant,
  array('obj' => Variant,
        'node' => array(Object, 'null_object'),
        'options' => array(Int64, '0')));
f('dom_document_validate', Variant,
  array('obj' => Variant));
f('dom_document_xinclude', Variant,
  array('obj' => Variant,
        'options' => array(Int64, '0')));
f('dom_document_save_html', Variant,
  array('obj' => Variant));
f('dom_document_save_html_file', Variant,
  array('obj' => Variant,
        'file' => String));
f('dom_document_schema_validate_file', Variant,
  array('obj' => Variant,
        'filename' => String));
f('dom_document_schema_validate_xml', Variant,
  array('obj' => Variant,
        'source' => String));
f('dom_document_relaxNG_validate_file', Variant,
  array('obj' => Variant,
        'filename' => String));
f('dom_document_relaxNG_validate_xml', Variant,
  array('obj' => Variant,
        'source' => String));

/* domnode methods */
f('dom_node_insert_before', Variant,
  array('obj' => Variant,
        'newnode' => Object,
        'refnode' => array(Object, 'null')));
f('dom_node_replace_child', Variant,
  array('obj' => Variant,
        'newChildObj' => Object,
        'oldChildObj' => Object));
f('dom_node_remove_child', Variant,
  array('obj' => Variant,
        'node' => Object));
f('dom_node_append_child', Variant,
  array('obj' => Variant,
        'newnode' => Object));
f('dom_node_has_child_nodes', Variant,
  array('obj' => Variant));
f('dom_node_clone_node', Variant,
  array('obj' => Variant,
        'deep' => array(Boolean, 'false')));
f('dom_node_normalize', Variant,
  array('obj' => Variant));
f('dom_node_is_supported', Variant,
  array('obj' => Variant,
        'feature' => String,
        'version' => String));
f('dom_node_has_attributes', Variant,
  array('obj' => Variant));
f('dom_node_is_same_node', Variant,
  array('obj' => Variant,
        'node' => Object));
f('dom_node_lookup_prefix', Variant,
  array('obj' => Variant,
        'prefix' => String));
f('dom_node_is_default_namespace', Variant,
  array('obj' => Variant,
        'namespaceURI' => String));
f('dom_node_lookup_namespace_uri', Variant,
  array('obj' => Variant,
        'namespaceURI' => String));

/* domnodelist methods */
f('dom_nodelist_item', Variant,
  array('obj' => Variant,
        'index' => Int64));

/* domnamednodemap methods */
f('dom_namednodemap_get_named_item', Variant,
  array('obj' => Variant,
        'name' => String));
f('dom_namednodemap_item', Variant,
  array('obj' => Variant,
        'index' => Int64));
f('dom_namednodemap_get_named_item_ns', Variant,
  array('obj' => Variant,
        'namespaceURI' => String,
        'localName' => String));

/* domcharacterdata methods */
f('dom_characterdata_substring_data', Variant,
  array('obj' => Variant,
        'offset' => Int64,
        'count' => Int64));
f('dom_characterdata_append_data', Variant,
  array('obj' => Variant,
        'arg' => String));
f('dom_characterdata_insert_data', Variant,
  array('obj' => Variant,
        'offset' => Int64,
        'data' => String));
f('dom_characterdata_delete_data', Variant,
  array('obj' => Variant,
        'offset' => Int64,
        'count' => Int64));
f('dom_characterdata_replace_data', Variant,
  array('obj' => Variant,
        'offset' => Int64,
        'count' => Int64,
        'data' => String));

/* domattr methods */
f('dom_attr_is_id', Variant,
  array('obj' => Variant));

/* domelement methods */
f('dom_element_get_attribute', Variant,
  array('obj' => Variant,
        'name' => String));
f('dom_element_set_attribute', Variant,
  array('obj' => Variant,
        'name' => String,
        'value' => String));
f('dom_element_remove_attribute', Variant,
  array('obj' => Variant,
        'name' => String));
f('dom_element_get_attribute_node', Variant,
  array('obj' => Variant,
        'name' => String));
f('dom_element_set_attribute_node', Variant,
  array('obj' => Variant,
        'newAttr' => Object));
f('dom_element_remove_attribute_node', Variant,
  array('obj' => Variant,
        'oldAttr' => Object));
f('dom_element_get_elements_by_tag_name', Variant,
  array('obj' => Variant,
        'name' => String));
f('dom_element_get_attribute_ns', Variant,
  array('obj' => Variant,
        'namespaceURI' => String,
        'localName' => String));
f('dom_element_set_attribute_ns', Variant,
  array('obj' => Variant,
        'namespaceURI' => String,
        'name' => String,
        'value' => String));
f('dom_element_remove_attribute_ns', Variant,
  array('obj' => Variant,
        'namespaceURI' => String,
        'localName' => String));
f('dom_element_get_attribute_node_ns', Variant,
  array('obj' => Variant,
        'namespaceURI' => String,
        'localName' => String));
f('dom_element_set_attribute_node_ns', Variant,
  array('obj' => Variant,
        'newAttr' => Object));
f('dom_element_get_elements_by_tag_name_ns', Variant,
  array('obj' => Variant,
        'namespaceURI' => String,
        'localName' => String));
f('dom_element_has_attribute', Variant,
  array('obj' => Variant,
        'name' => String));
f('dom_element_has_attribute_ns', Variant,
  array('obj' => Variant,
        'namespaceURI' => String,
        'localName' => String));
f('dom_element_set_id_attribute', Variant,
  array('obj' => Variant,
        'name' => String,
        'isId' => Boolean));
f('dom_element_set_id_attribute_ns', Variant,
  array('obj' => Variant,
        'namespaceURI' => String,
        'localName' => String,
        'isId' => Boolean));
f('dom_element_set_id_attribute_node', Variant,
  array('obj' => Variant,
        'idAttr' => Object,
        'isId' => Boolean));

/* domtext methods */
f('dom_text_split_text', Variant,
  array('obj' => Variant,
        'offset' => Int64));
f('dom_text_is_whitespace_in_element_content', Variant,
  array('obj' => Variant));

/* xpath methods */
f('dom_xpath_register_ns', Variant,
  array('obj' => Variant,
        'prefix' => String,
        'uri' => String));
f('dom_xpath_query', Variant,
  array('obj' => Variant,
        'expr' => String,
        'context' => array(Object, 'null_object')));
f('dom_xpath_evaluate', Variant,
  array('obj' => Variant,
        'expr' => String,
        'context' => array(Object, 'null_object')));
f('dom_xpath_register_php_functions', Variant,
  array('obj' => Variant,
        'funcs' => array(Variant, 'null')));
