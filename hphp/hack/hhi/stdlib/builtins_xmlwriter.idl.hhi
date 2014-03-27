<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
function xmlwriter_open_memory() { }
function xmlwriter_open_uri($uri) { }
function xmlwriter_set_indent_string($xmlwriter, $indentstring) { }
function xmlwriter_set_indent($xmlwriter, $indent) { }
function xmlwriter_start_document($xmlwriter, $version = "1.0", $encoding = null, $standalone = null) { }
function xmlwriter_start_element($xmlwriter, $name) { }
function xmlwriter_start_element_ns($xmlwriter, $prefix, $name, $uri) { }
function xmlwriter_write_element_ns($xmlwriter, $prefix, $name, $uri, $content = null) { }
function xmlwriter_write_element($xmlwriter, $name, $content = null) { }
function xmlwriter_end_element($xmlwriter) { }
function xmlwriter_full_end_element($xmlwriter) { }
function xmlwriter_start_attribute_ns($xmlwriter, $prefix, $name, $uri) { }
function xmlwriter_start_attribute($xmlwriter, $name) { }
function xmlwriter_write_attribute_ns($xmlwriter, $prefix, $name, $uri, $content) { }
function xmlwriter_write_attribute($xmlwriter, $name, $value) { }
function xmlwriter_end_attribute($xmlwriter) { }
function xmlwriter_start_cdata($xmlwriter) { }
function xmlwriter_write_cdata($xmlwriter, $content) { }
function xmlwriter_end_cdata($xmlwriter) { }
function xmlwriter_start_comment($xmlwriter) { }
function xmlwriter_write_comment($xmlwriter, $content) { }
function xmlwriter_end_comment($xmlwriter) { }
function xmlwriter_end_document($xmlwriter) { }
function xmlwriter_start_pi($xmlwriter, $target) { }
function xmlwriter_write_pi($xmlwriter, $target, $content) { }
function xmlwriter_end_pi($xmlwriter) { }
function xmlwriter_text($xmlwriter, $content) { }
function xmlwriter_write_raw($xmlwriter, $content) { }
function xmlwriter_start_dtd($xmlwriter, $qualifiedname, $publicid = null, $systemid = null) { }
function xmlwriter_write_dtd($xmlwriter, $name, $publicid = null, $systemid = null, $subset = null) { }
function xmlwriter_start_dtd_element($xmlwriter, $qualifiedname) { }
function xmlwriter_write_dtd_element($xmlwriter, $name, $content) { }
function xmlwriter_end_dtd_element($xmlwriter) { }
function xmlwriter_start_dtd_attlist($xmlwriter, $name) { }
function xmlwriter_write_dtd_attlist($xmlwriter, $name, $content) { }
function xmlwriter_end_dtd_attlist($xmlwriter) { }
function xmlwriter_start_dtd_entity($xmlwriter, $name, $isparam) { }
function xmlwriter_write_dtd_entity($xmlwriter, $name, $content, $pe = false, $publicid = null, $systemid = null, $ndataid = null) { }
function xmlwriter_end_dtd_entity($xmlwriter) { }
function xmlwriter_end_dtd($xmlwriter) { }
function xmlwriter_flush($xmlwriter, $empty = true) { }
function xmlwriter_output_memory($xmlwriter, $flush = true) { }
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
  public function startAttributens($prefix, $name, $uri) { }
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
