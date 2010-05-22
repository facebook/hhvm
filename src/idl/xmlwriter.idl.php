<?php

include_once 'base.php';

p("#include <libxml/tree.h>");
p("#include <libxml/xmlwriter.h>");
p("#include <libxml/uri.h>");
p("#include <runtime/base/file/file.h>");

///////////////////////////////////////////////////////////////////////////////
// function style

f('xmlwriter_open_memory', Variant);

f('xmlwriter_open_uri', Resource,
  array('uri' => String));

f('xmlwriter_set_indent_string', Boolean,
  array('xmlwriter' => Resource,
        'indentString' => String));

f('xmlwriter_set_indent', Boolean,
  array('xmlwriter' => Resource,
        'indent' => Boolean));

f('xmlwriter_start_document', Boolean,
  array('xmlwriter' => Resource,
        'version' => array(String, '"1.0"'),
        'encoding' => array(String, 'null_string'),
        'standalone' => array(String, 'null_string')));

f('xmlwriter_start_element', Boolean,
  array('xmlwriter' => Resource,
        'name' => String));

f('xmlwriter_start_element_ns', Boolean,
  array('xmlwriter' => Resource,
        'prefix' => String,
        'name' => String,
        'uri' => String));

f('xmlwriter_write_element_ns', Boolean,
  array('xmlwriter' => Resource,
        'prefix' => String,
        'name' => String,
        'uri' => String,
        'content' => array(String, 'null_string')));

f('xmlwriter_write_element', Boolean,
  array('xmlwriter' => Resource,
        'name' => String,
        'content' => array(String, 'null_string')));

f('xmlwriter_end_element', Boolean,
  array('xmlwriter' => Resource));

f('xmlwriter_full_end_element', Boolean,
  array('xmlwriter' => Resource));

f('xmlwriter_start_attribute_ns', Boolean,
  array('xmlwriter' => Resource,
        'prefix' => String,
        'name' => String,
        'uri' => String));

f('xmlwriter_start_attribute', Boolean,
  array('xmlwriter' => Resource,
        'name' => String));

f('xmlwriter_write_attribute_ns', Boolean,
  array('xmlwriter' => Resource,
        'prefix' => String,
        'name' => String,
        'uri' => String,
        'content' => String));

f('xmlwriter_write_attribute', Boolean,
  array('xmlwriter' => Resource,
        'name' => String,
        'value' => String));

f('xmlwriter_end_attribute', Boolean,
  array('xmlwriter' => Resource));

f('xmlwriter_start_cdata', Boolean,
  array('xmlwriter' => Resource));

f('xmlwriter_write_cdata', Boolean,
  array('xmlwriter' => Resource,
        'content' => String));

f('xmlwriter_end_cdata', Boolean,
  array('xmlwriter' => Resource));

f('xmlwriter_start_comment', Boolean,
  array('xmlwriter' => Resource));

f('xmlwriter_write_comment', Boolean,
  array('xmlwriter' => Resource,
        'content' => String));

f('xmlwriter_end_comment', Boolean,
  array('xmlwriter' => Resource));

f('xmlwriter_end_document', Boolean,
  array('xmlwriter' => Resource));

f('xmlwriter_start_pi', Boolean,
  array('xmlwriter' => Resource,
        'target' => String));

f('xmlwriter_write_pi', Boolean,
  array('xmlwriter' => Resource,
        'target' => String,
        'content' => String));

f('xmlwriter_end_pi', Boolean,
  array('xmlwriter' => Resource));

f('xmlwriter_text', Boolean,
  array('xmlwriter' => Resource,
        'content' => String));

f('xmlwriter_write_raw', Boolean,
  array('xmlwriter' => Resource,
        'content' => String));

f('xmlwriter_start_dtd', Boolean,
  array('xmlwriter' => Resource,
        'qualifiedName' => String,
        'publicId' => array(String, 'null_string'),
        'systemId' => array(String, 'null_string')));

f('xmlwriter_write_dtd', Boolean,
  array('xmlwriter' => Resource,
        'name' => String,
        'publicId' => array(String, 'null_string'),
        'systemId' => array(String, 'null_string'),
        'subset' => array(String, 'null_string')));

f('xmlwriter_start_dtd_element', Boolean,
  array('xmlwriter' => Resource,
        'qualifiedName' => String));

f('xmlwriter_write_dtd_element', Boolean,
  array('xmlwriter' => Resource,
        'name' => String,
        'content' => String));

f('xmlwriter_end_dtd_element', Boolean,
  array('xmlwriter' => Resource));

f('xmlwriter_start_dtd_attlist', Boolean,
  array('xmlwriter' => Resource,
        'name' => String));

f('xmlwriter_write_dtd_attlist', Boolean,
  array('xmlwriter' => Resource,
        'name' => String,
        'content' => String));

f('xmlwriter_end_dtd_attlist', Boolean,
  array('xmlwriter' => Resource));

f('xmlwriter_start_dtd_entity', Boolean,
  array('xmlwriter' => Resource,
        'name' => String,
        'isparam' => Boolean));

f('xmlwriter_write_dtd_entity', Boolean,
  array('xmlwriter' => Resource,
        'name' => String,
        'content' => String,
        'pe' => array(Boolean, 'false'),
        'publicId' => array(String, 'null_string'),
        'systemId' => array(String, 'null_string'),
        'ndataid' => array(String, 'null_string')));

f('xmlwriter_end_dtd_entity', Boolean,
  array('xmlwriter' => Resource));

f('xmlwriter_end_dtd', Boolean,
  array('xmlwriter' => Resource));

f('xmlwriter_flush', Variant,
  array('xmlwriter' => Resource,
        'empty' => array(Boolean, 'true')));

f('xmlwriter_output_memory', String,
  array('xmlwriter' => Resource,
        'flush' => array(Boolean, 'true')));

///////////////////////////////////////////////////////////////////////////////
// class style

c('xmlwriter', null, array(),
  array(
    m(PublicMethod, "__construct"),

    m(PublicMethod, 'openMemory', Boolean),
    m(PublicMethod, 'openURI', Boolean,
      array('uri' => String)),
    m(PublicMethod, 'setIndentString', Boolean,
      array('indentString' => String)),
    m(PublicMethod, 'setIndent', Boolean,
      array('indent' => Boolean)),
    m(PublicMethod, 'startDocument', Boolean,
      array('version' => array(String, '"1.0"'),
            'encoding' => array(String, 'null_string'),
            'standalone' => array(String, 'null_string'))),
    m(PublicMethod, 'startElement', Boolean,
      array('name' => String)),
    m(PublicMethod, 'startElementNS', Boolean,
      array('prefix' => String,
            'name' => String,
            'uri' => String)),
    m(PublicMethod, 'writeElementNS', Boolean,
      array('prefix' => String,
            'name' => String,
            'uri' => String,
            'content' => array(String, 'null_string'))),
    m(PublicMethod, 'writeElement', Boolean,
      array('name' => String,
            'content' => array(String, 'null_string'))),
    m(PublicMethod, 'endElement', Boolean),
    m(PublicMethod, 'fullEndElement', Boolean),
    m(PublicMethod, 'startAttributeNS', Boolean,
      array('prefix' => String,
            'name' => String,
            'uri' => String)),
    m(PublicMethod, 'startAttribute', Boolean,
      array('name' => String)),
    m(PublicMethod, 'writeAttributeNS', Boolean,
      array('prefix' => String,
            'name' => String,
            'uri' => String,
            'content' => String)),
    m(PublicMethod, 'writeAttribute', Boolean,
      array('name' => String,
            'value' => String)),
    m(PublicMethod, 'endAttribute', Boolean),
    m(PublicMethod, 'startCData', Boolean),
    m(PublicMethod, 'writeCData', Boolean,
      array('content' => String)),
    m(PublicMethod, 'endCData', Boolean),
    m(PublicMethod, 'startComment', Boolean),
    m(PublicMethod, 'writeComment', Boolean,
      array('content' => String)),
    m(PublicMethod, 'endComment', Boolean),
    m(PublicMethod, 'endDocument', Boolean),
    m(PublicMethod, 'startPI', Boolean,
      array('target' => String)),
    m(PublicMethod, 'writePI', Boolean,
      array('target' => String,
            'content' => String)),
    m(PublicMethod, 'endPI', Boolean),
    m(PublicMethod, 'text', Boolean,
      array('content' => String)),
    m(PublicMethod, 'writeRaw', Boolean,
      array('content' => String)),
    m(PublicMethod, 'startDTD', Boolean,
      array('qualifiedName' => String,
            'publicId' => array(String, 'null_string'),
            'systemId' => array(String, 'null_string'))),
    m(PublicMethod, 'writeDTD', Boolean,
      array('name' => String,
            'publicId' => array(String, 'null_string'),
            'systemId' => array(String, 'null_string'),
            'subset' => array(String, 'null_string'))),
    m(PublicMethod, 'startDTDElement', Boolean,
      array('qualifiedName' => String)),
    m(PublicMethod, 'writeDTDElement', Boolean,
      array('name' => String,
            'content' => String)),
    m(PublicMethod, 'endDTDElement', Boolean),
    m(PublicMethod, 'startDTDAttlist', Boolean,
      array('name' => String)),
    m(PublicMethod, 'writeDTDAttlist', Boolean,
      array('name' => String,
            'content' => String)),
    m(PublicMethod, 'endDTDAttlist', Boolean),
    m(PublicMethod, 'startDTDEntity', Boolean,
      array('name' => String,
            'isparam' => Boolean)),
    m(PublicMethod, 'writeDTDEntity', Boolean,
      array('name' => String,
            'content' => String,
            'pe' => array(Boolean, 'false'),
            'publicId' => array(String, 'null_string'),
            'systemId' => array(String, 'null_string'),
            'ndataid' => array(String, 'null_string'))),
    m(PublicMethod, 'endDTDEntity', Boolean),
    m(PublicMethod, 'endDTD', Boolean),
    m(PublicMethod, 'flush', Variant,
      array('empty' => array(Boolean, 'true'))),
    m(PublicMethod, 'outputMemory', String,
      array('flush' => array(Boolean, 'true'))),
    ),

  array(), // constants
  "\n public:".
  "\n  SmartObject<File>  m_uri;".
  "\n private:".
  "\n  xmlTextWriterPtr   m_ptr;".
  "\n  xmlBufferPtr       m_output;".
  "\n  xmlOutputBufferPtr m_uri_output;"
  );

///////////////////////////////////////////////////////////////////////////////
