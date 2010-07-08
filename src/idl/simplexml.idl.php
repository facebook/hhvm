<?php

include_once 'base.php';

pre("#include <runtime/ext/ext_simplexml_include.h>");

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
f('libxml_disable_entity_loader', Boolean,
  array('disable' => array(Boolean, 'true')));

///////////////////////////////////////////////////////////////////////////////

c('SimpleXMLElement', null,
  array('ArrayAccess', 'IteratorAggregate', 'Countable'),
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
    m(PublicMethod, 'getIterator', Variant),
    m(PublicMethod, 'count', Int64),
    m(PublicMethod, 'xpath', Variant,
      array('path' => String)),
    m(PublicMethod, 'registerXPathNamespace', Boolean,
      array('prefix' => String,
            'ns' => String)),
    m(PublicMethod, 'asXML', Variant,
      array('filename' => array(String, '""'))),
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
    m(PublicMethod, "__get", Variant,
      array('name' => Variant)),
    m(PublicMethod, "__set", Variant,
      array('name' => Variant,
            'value' => Variant)),
    m(PublicMethod, "__unset", Variant,
      array('name' => Variant)),
    ),

  array(), // constants
  "\n".
  " public:\n".
  "  Object m_doc;\n".
  "  xmlNodePtr m_node;\n".
  "  Variant m_children;\n".
  "  Variant m_attributes;\n".
  "  bool m_is_text;\n".
  "  bool m_free_text;\n".
  "  bool m_is_attribute;\n".
  "  bool m_is_children;\n".
  "  bool m_is_property;\n".
  "  bool m_is_root;\n".
  "  virtual Array o_toArray() const;\n".
  "  virtual int64 o_toInt64() const;\n".
  "  virtual Variant &___lval(Variant v_name);\n".
  " private:\n".
  "  xmlXPathContextPtr m_xpath;"
  );

c('LibXMLError', null, array(),
  array(m(PublicMethod, '__construct')),
  array(), // constants
  "");

c('SimpleXMLElementIterator', null,
  array('Iterator', 'Sweepable' => 'internal'),
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
  "  void reset_iterator(c_simplexmlelement *parent);\n".
  "\n".
  "  c_simplexmlelement *m_parent;\n".
  "  ArrayIter *m_iter1;\n".
  "  ArrayIter *m_iter2;\n".
  "  Object     m_temp;"
 );
