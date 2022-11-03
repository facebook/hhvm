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
function xmlwriter_open_memory(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_open_uri(string $uri): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_set_indent_string(
  resource $xmlwriter,
  $indentstring,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_set_indent(
  resource $xmlwriter,
  bool $indent,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_start_document(
  resource $xmlwriter,
  $version = "1.0",
  $encoding = null,
  $standalone = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_start_element(
  resource $xmlwriter,
  string $name,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_start_element_ns(
  resource $xmlwriter,
  $prefix,
  string $name,
  $uri,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_write_element_ns(
  resource $xmlwriter,
  $prefix,
  string $name,
  $uri,
  $content = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_write_element(
  resource $xmlwriter,
  string $name,
  $content = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_end_element(
  resource $xmlwriter,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_full_end_element(
  resource $xmlwriter,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_start_attribute_ns(
  resource $xmlwriter,
  string $prefix,
  string $name,
  string $uri,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_start_attribute(
  resource $xmlwriter,
  string $name,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_write_attribute_ns(
  resource $xmlwriter,
  string $prefix,
  string $name,
  string $uri,
  string $content,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_write_attribute(
  resource $xmlwriter,
  string $name,
  string $value,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_end_attribute(
  resource $xmlwriter,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_start_cdata(
  resource $xmlwriter,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_write_cdata(
  resource $xmlwriter,
  string $content,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_end_cdata(
  resource $xmlwriter,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_start_comment(
  resource $xmlwriter,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_write_comment(
  resource $xmlwriter,
  string $content,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_end_comment(
  resource $xmlwriter,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_end_document(
  resource $xmlwriter,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_start_pi(
  resource $xmlwriter,
  string $target,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_write_pi(
  resource $xmlwriter,
  string $target,
  string $content,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_end_pi(resource $xmlwriter): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_text(
  resource $xmlwriter,
  string $content,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_write_raw(
  resource $xmlwriter,
  string $content,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_start_dtd(
  resource $xmlwriter,
  string $qualifiedname,
  $publicid = null,
  $systemid = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_write_dtd(
  resource $xmlwriter,
  string $name,
  $publicid = null,
  $systemid = null,
  $subset = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_start_dtd_element(
  resource $xmlwriter,
  string $qualifiedname,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_write_dtd_element(
  resource $xmlwriter,
  string $name,
  string $content,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_end_dtd_element(
  resource $xmlwriter,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_start_dtd_attlist(
  resource $xmlwriter,
  string $name,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_write_dtd_attlist(
  resource $xmlwriter,
  string $name,
  string $content,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_end_dtd_attlist(
  resource $xmlwriter,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_start_dtd_entity(
  resource $xmlwriter,
  string $name,
  bool $isparam,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_write_dtd_entity(
  resource $xmlwriter,
  string $name,
  string $content,
  bool $pe = false,
  string $publicid = "",
  string $systemid = "",
  string $ndataid = "",
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_end_dtd_entity(
  resource $xmlwriter,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_end_dtd(resource $xmlwriter): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_flush(
  resource $xmlwriter,
  $empty = true,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_output_memory(
  resource $xmlwriter,
  $flush = true,
): HH\FIXME\MISSING_RETURN_TYPE;

class XMLWriter {
  public function __construct();
  public function openMemory(): HH\FIXME\MISSING_RETURN_TYPE;
  public function openURI(string $uri): HH\FIXME\MISSING_RETURN_TYPE;
  public function setIndentString(
    string $indentstring,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setIndent(bool $indent): HH\FIXME\MISSING_RETURN_TYPE;
  public function startDocument(
    $version = "1.0",
    $encoding = null,
    $standalone = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function startElement(string $name): HH\FIXME\MISSING_RETURN_TYPE;
  public function startElementNS(
    $prefix,
    string $name,
    $uri,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function writeElementNS(
    $prefix,
    string $name,
    $uri,
    $content = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function writeElement(
    string $name,
    $content = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function endElement(): HH\FIXME\MISSING_RETURN_TYPE;
  public function fullEndElement(): HH\FIXME\MISSING_RETURN_TYPE;
  public function startAttributeNS(
    string $prefix,
    string $name,
    string $uri,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function startAttribute(string $name): HH\FIXME\MISSING_RETURN_TYPE;
  public function writeAttributeNS(
    string $prefix,
    string $name,
    string $uri,
    string $content,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function writeAttribute(
    string $name,
    string $value,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function endAttribute(): HH\FIXME\MISSING_RETURN_TYPE;
  public function startCData(): HH\FIXME\MISSING_RETURN_TYPE;
  public function writeCData(string $content): HH\FIXME\MISSING_RETURN_TYPE;
  public function endCData(): HH\FIXME\MISSING_RETURN_TYPE;
  public function startComment(): HH\FIXME\MISSING_RETURN_TYPE;
  public function writeComment(string $content): HH\FIXME\MISSING_RETURN_TYPE;
  public function endComment(): HH\FIXME\MISSING_RETURN_TYPE;
  public function endDocument(): HH\FIXME\MISSING_RETURN_TYPE;
  public function startPI(string $target): HH\FIXME\MISSING_RETURN_TYPE;
  public function writePI(
    string $target,
    string $content,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function endPI(): HH\FIXME\MISSING_RETURN_TYPE;
  public function text(string $content): HH\FIXME\MISSING_RETURN_TYPE;
  public function writeRaw(string $content): HH\FIXME\MISSING_RETURN_TYPE;
  public function startDTD(
    string $qualifiedname,
    $publicid = null,
    $systemid = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function writeDTD(
    string $name,
    $publicid = null,
    $systemid = null,
    $subset = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function startDTDElement(
    string $qualifiedname,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function writeDTDElement(
    string $name,
    string $content,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function endDTDElement(): HH\FIXME\MISSING_RETURN_TYPE;
  public function startDTDAttlist(string $name): HH\FIXME\MISSING_RETURN_TYPE;
  public function writeDTDAttlist(
    string $name,
    string $content,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function endDTDAttlist(): HH\FIXME\MISSING_RETURN_TYPE;
  public function startDTDEntity(
    string $name,
    bool $isparam,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function writeDTDEntity(
    string $name,
    string $content,
    bool $pe = false,
    string $publicid = "",
    string $systemid = "",
    string $ndataid = "",
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function endDTDEntity(): HH\FIXME\MISSING_RETURN_TYPE;
  public function endDTD(): HH\FIXME\MISSING_RETURN_TYPE;
  public function flush($empty = true): HH\FIXME\MISSING_RETURN_TYPE;
  public function outputMemory($flush = true): HH\FIXME\MISSING_RETURN_TYPE;
}
