<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib>>
function xmlwriter_open_memory();
<<__PHPStdLib>>
function xmlwriter_open_uri(string $uri);
<<__PHPStdLib>>
function xmlwriter_set_indent_string(resource $xmlwriter, $indentstring);
<<__PHPStdLib>>
function xmlwriter_set_indent(resource $xmlwriter, bool $indent);
<<__PHPStdLib>>
function xmlwriter_start_document(resource $xmlwriter, $version = "1.0", $encoding = null, $standalone = null);
<<__PHPStdLib>>
function xmlwriter_start_element(resource $xmlwriter, string $name);
<<__PHPStdLib>>
function xmlwriter_start_element_ns(resource $xmlwriter, $prefix, string $name, $uri);
<<__PHPStdLib>>
function xmlwriter_write_element_ns(resource $xmlwriter, $prefix, string $name, $uri, $content = null);
<<__PHPStdLib>>
function xmlwriter_write_element(resource $xmlwriter, string $name, $content = null);
<<__PHPStdLib>>
function xmlwriter_end_element(resource $xmlwriter);
<<__PHPStdLib>>
function xmlwriter_full_end_element(resource $xmlwriter);
<<__PHPStdLib>>
function xmlwriter_start_attribute_ns(resource $xmlwriter, string $prefix, string $name, string $uri);
<<__PHPStdLib>>
function xmlwriter_start_attribute(resource $xmlwriter, string $name);
<<__PHPStdLib>>
function xmlwriter_write_attribute_ns(resource $xmlwriter, string $prefix, string $name, string $uri, string $content);
<<__PHPStdLib>>
function xmlwriter_write_attribute(resource $xmlwriter, string $name, string $value);
<<__PHPStdLib>>
function xmlwriter_end_attribute(resource $xmlwriter);
<<__PHPStdLib>>
function xmlwriter_start_cdata(resource $xmlwriter);
<<__PHPStdLib>>
function xmlwriter_write_cdata(resource $xmlwriter, string $content);
<<__PHPStdLib>>
function xmlwriter_end_cdata(resource $xmlwriter);
<<__PHPStdLib>>
function xmlwriter_start_comment(resource $xmlwriter);
<<__PHPStdLib>>
function xmlwriter_write_comment(resource $xmlwriter, string $content);
<<__PHPStdLib>>
function xmlwriter_end_comment(resource $xmlwriter);
<<__PHPStdLib>>
function xmlwriter_end_document(resource $xmlwriter);
<<__PHPStdLib>>
function xmlwriter_start_pi(resource $xmlwriter, string $target);
<<__PHPStdLib>>
function xmlwriter_write_pi(resource $xmlwriter, string $target, string $content);
<<__PHPStdLib>>
function xmlwriter_end_pi(resource $xmlwriter);
<<__PHPStdLib>>
function xmlwriter_text(resource $xmlwriter, string $content);
<<__PHPStdLib>>
function xmlwriter_write_raw(resource $xmlwriter, string $content);
<<__PHPStdLib>>
function xmlwriter_start_dtd(resource $xmlwriter, string $qualifiedname, $publicid = null, $systemid = null);
<<__PHPStdLib>>
function xmlwriter_write_dtd(resource $xmlwriter, string $name, $publicid = null, $systemid = null, $subset = null);
<<__PHPStdLib>>
function xmlwriter_start_dtd_element(resource $xmlwriter, string $qualifiedname);
<<__PHPStdLib>>
function xmlwriter_write_dtd_element(resource $xmlwriter, string $name, string $content);
<<__PHPStdLib>>
function xmlwriter_end_dtd_element(resource $xmlwriter);
<<__PHPStdLib>>
function xmlwriter_start_dtd_attlist(resource $xmlwriter, string $name);
<<__PHPStdLib>>
function xmlwriter_write_dtd_attlist(resource $xmlwriter, string $name, string $content);
<<__PHPStdLib>>
function xmlwriter_end_dtd_attlist(resource $xmlwriter);
<<__PHPStdLib>>
function xmlwriter_start_dtd_entity(resource $xmlwriter, string $name, bool $isparam);
<<__PHPStdLib>>
function xmlwriter_write_dtd_entity(resource $xmlwriter, string $name, string $content, bool $pe = false, string $publicid = "", string $systemid = "", string $ndataid = "");
<<__PHPStdLib>>
function xmlwriter_end_dtd_entity(resource $xmlwriter);
<<__PHPStdLib>>
function xmlwriter_end_dtd(resource $xmlwriter);
<<__PHPStdLib>>
function xmlwriter_flush(resource $xmlwriter, $empty = true);
<<__PHPStdLib>>
function xmlwriter_output_memory(resource $xmlwriter, $flush = true);

class XMLWriter {
  public function __construct() { }
  public function openMemory() { }
  public function openURI($uri) { }
  public function setIndentString($indentstring) { }
  public function setIndent($indent) { }
  public function startDocument($version = "1.0", $encoding = null, $standalone = null) { }
  public function startElement($name) { }
  public function startElementNS($prefix, $name, $uri) { }
  public function writeElementNS($prefix, $name, $uri, $content = null) { }
  public function writeElement($name, $content = null) { }
  public function endElement() { }
  public function fullEndElement() { }
  public function startAttributeNS($prefix, $name, $uri) { }
  public function startAttribute($name) { }
  public function writeAttributeNS($prefix, $name, $uri, $content) { }
  public function writeAttribute($name, $value) { }
  public function endAttribute() { }
  public function startCData() { }
  public function writeCData($content) { }
  public function endCData() { }
  public function startComment() { }
  public function writeComment($content) { }
  public function endComment() { }
  public function endDocument() { }
  public function startPI($target) { }
  public function writePI($target, $content) { }
  public function endPI() { }
  public function text($content) { }
  public function writeRaw($content) { }
  public function startDTD($qualifiedname, $publicid = null, $systemid = null) { }
  public function writeDTD($name, $publicid = null, $systemid = null, $subset = null) { }
  public function startDTDElement($qualifiedname) { }
  public function writeDTDElement($name, $content) { }
  public function endDTDElement() { }
  public function startDTDAttlist($name) { }
  public function writeDTDAttlist($name, $content) { }
  public function endDTDAttlist() { }
  public function startDTDEntity($name, $isparam) { }
  public function writeDTDEntity($name, $content, $pe = false, $publicid = null, $systemid = null, $ndataid = null) { }
  public function endDTDEntity() { }
  public function endDTD() { }
  public function flush($empty = true) { }
  public function outputMemory($flush = true) { }
}
