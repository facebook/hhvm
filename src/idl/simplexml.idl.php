<?php

include_once 'base.php';

p("#include <cpp/ext/ext_simplexml_include.h>");

///////////////////////////////////////////////////////////////////////////////

f('simplexml_load_string', Variant,
  array('data' => String,
        'class_name' => array(String, '"SimpleXMLElement"'),
        'options' => array(Int64, '0'),
        'ns' => array(String, '""'),
        'is_prefix' => array(Boolean, 'false')));

f('simplexml_load_file', Variant,
  array('filename' => String,
        'class_name' => array(String, '"SimpleXMLElement"'),
        'options' => array(Int64, '0'),
        'ns' => array(String, '""'),
        'is_prefix' => array(Boolean, 'false')));

f('libxml_get_errors', Variant);
f('libxml_get_last_error', Variant);
f('libxml_clear_errors');
f('libxml_use_internal_errors', Boolean,
  array('use_errors' => array(Variant, 'null_variant')));
f('libxml_set_streams_context', NULL,
  array('streams_context' => Resource));

///////////////////////////////////////////////////////////////////////////////

c('SimpleXMLElement', null, array('ArrayAccess'),
  array(
    m(PublicMethod, '__construct', NULL,
      array('data' => String,
            'options' => array(Int64, '0'),
            'data_is_url' => array(Boolean, 'false'),
            'ns' => array(String, '""'),
            'is_prefix' => array(Boolean, 'false'))),
    m(PublicMethod, 'offsetExists', Boolean,
      array('index' => Variant)),
    m(PublicMethod, 'offsetGet', Variant,
      array('index' => Variant)),
    m(PublicMethod, 'offsetSet', NULL,
      array('index' => Variant,
            'newvalue' => Variant)),
    m(PublicMethod, 'offsetUnset', NULL,
      array('index' => Variant)),
    m(PublicMethod, 'xpath', Variant,
      array('path' => String)),
    m(PublicMethod, 'registerXPathNamespace', Boolean,
      array('prefix' => String,
            'ns' => String)),
    m(PublicMethod, 'asXML', Variant,
      array('filename' => String)),
    m(PublicMethod, 'getNamespaces', StringMap,
      array('recursive' => array(Boolean, 'false'))),
    m(PublicMethod, 'getDocNamespaces', StringMap,
      array('recursive' => array(Boolean, 'false'))),
    m(PublicMethod, 'children', Object,
      array('ns' => array(String, '""'),
            'is_prefix' => array(Boolean, 'false'))),
    m(PublicMethod, 'getName', String),
    m(PublicMethod, 'attributes', Object,
      array('ns' => array(String, '""'),
            'is_prefix' => array(Boolean, 'false'))),
    m(PublicMethod, 'addChild', Variant,
      array('qName' => String,
            'value' => array(String, 'null_string'),
            'ns' => array(String, 'null_string'))),
    m(PublicMethod, 'addAttribute', NULL,
      array('qName' => String,
            'value' => array(String, 'null_string'),
            'ns' => array(String, 'null_string'))),
    m(PublicMethod, '__toString', String),
    ),

  array(), // constants
  "\n".
  " public:\n".
  "  Object m_doc;\n".
  "  xmlNodePtr m_node;\n".
  "  Array m_attributes;\n".
  "  virtual Array o_toIterArray(const char *context);\n".
  " private:\n".
  "  bool m_is_attribute;\n".
  "  bool m_is_children;\n".
  "  xmlXPathContextPtr m_xpath;"
  );

c('LibXMLError', null, array(),
  array(m(PublicMethod, '__construct')),
  array(), // constants
  "");
