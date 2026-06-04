<?hh /* -*- php -*- */
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
  HH\FIXME\MISSING_PARAM_TYPE $indentstring,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_set_indent(
  resource $xmlwriter,
  bool $indent,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_start_document(
  resource $xmlwriter,
  HH\FIXME\MISSING_PARAM_TYPE $version = "1.0",
  HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
  HH\FIXME\MISSING_PARAM_TYPE $standalone = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_start_element(
  resource $xmlwriter,
  string $name,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_start_element_ns(
  resource $xmlwriter,
  HH\FIXME\MISSING_PARAM_TYPE $prefix,
  string $name,
  HH\FIXME\MISSING_PARAM_TYPE $uri,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_write_element_ns(
  resource $xmlwriter,
  HH\FIXME\MISSING_PARAM_TYPE $prefix,
  string $name,
  HH\FIXME\MISSING_PARAM_TYPE $uri,
  HH\FIXME\MISSING_PARAM_TYPE $content = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_write_element(
  resource $xmlwriter,
  string $name,
  HH\FIXME\MISSING_PARAM_TYPE $content = null,
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
function xmlwriter_end_cdata(resource $xmlwriter): HH\FIXME\MISSING_RETURN_TYPE;
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
  HH\FIXME\MISSING_PARAM_TYPE $publicid = null,
  HH\FIXME\MISSING_PARAM_TYPE $systemid = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_write_dtd(
  resource $xmlwriter,
  string $name,
  HH\FIXME\MISSING_PARAM_TYPE $publicid = null,
  HH\FIXME\MISSING_PARAM_TYPE $systemid = null,
  HH\FIXME\MISSING_PARAM_TYPE $subset = null,
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
  HH\FIXME\MISSING_PARAM_TYPE $empty = true,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function xmlwriter_output_memory(
  resource $xmlwriter,
  HH\FIXME\MISSING_PARAM_TYPE $flush = true,
): HH\FIXME\MISSING_RETURN_TYPE;

class XMLWriter {
  public function __construct();
  public function openMemory(): bool;
  public function openURI(string $uri): bool;
  public function setIndentString(
    string $indentstring,
  ): bool;
  public function setIndent(bool $indent): bool;
  public function startDocument(
    HH\FIXME\MISSING_PARAM_TYPE $version = "1.0",
    HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
    HH\FIXME\MISSING_PARAM_TYPE $standalone = null,
  ): bool;
  public function startElement(string $name): bool;
  public function startElementNS(
    HH\FIXME\MISSING_PARAM_TYPE $prefix,
    string $name,
    HH\FIXME\MISSING_PARAM_TYPE $uri,
  ): bool;
  public function writeElementNS(
    HH\FIXME\MISSING_PARAM_TYPE $prefix,
    string $name,
    HH\FIXME\MISSING_PARAM_TYPE $uri,
    HH\FIXME\MISSING_PARAM_TYPE $content = null,
  ): bool;
  public function writeElement(
    string $name,
    HH\FIXME\MISSING_PARAM_TYPE $content = null,
  ): bool;
  public function endElement(): bool;
  public function fullEndElement(): bool;
  public function startAttributeNS(
    string $prefix,
    string $name,
    string $uri,
  ): bool;
  public function startAttribute(string $name): bool;
  public function writeAttributeNS(
    string $prefix,
    string $name,
    string $uri,
    string $content,
  ): bool;
  public function writeAttribute(
    string $name,
    string $value,
  ): bool;
  public function endAttribute(): bool;
  public function startCData(): bool;
  public function writeCData(string $content): bool;
  public function endCData(): bool;
  public function startComment(): bool;
  public function writeComment(string $content): bool;
  public function endComment(): bool;
  public function endDocument(): bool;
  public function startPI(string $target): bool;
  public function writePI(
    string $target,
    string $content,
  ): bool;
  public function endPI(): bool;
  public function text(string $content): bool;
  public function writeRaw(string $content): bool;
  public function startDTD(
    string $qualifiedname,
    HH\FIXME\MISSING_PARAM_TYPE $publicid = null,
    HH\FIXME\MISSING_PARAM_TYPE $systemid = null,
  ): bool;
  public function writeDTD(
    string $name,
    HH\FIXME\MISSING_PARAM_TYPE $publicid = null,
    HH\FIXME\MISSING_PARAM_TYPE $systemid = null,
    HH\FIXME\MISSING_PARAM_TYPE $subset = null,
  ): bool;
  public function startDTDElement(
    string $qualifiedname,
  ): bool;
  public function writeDTDElement(
    string $name,
    string $content,
  ): bool;
  public function endDTDElement(): bool;
  public function startDTDAttlist(string $name): bool;
  public function writeDTDAttlist(
    string $name,
    string $content,
  ): bool;
  public function endDTDAttlist(): bool;
  public function startDTDEntity(
    string $name,
    bool $isparam,
  ): bool;
  public function writeDTDEntity(
    string $name,
    string $content,
    bool $pe = false,
    string $publicid = "",
    string $systemid = "",
    string $ndataid = "",
  ): bool;
  public function endDTDEntity(): bool;
  public function endDTD(): bool;
  public function flush(
    HH\FIXME\MISSING_PARAM_TYPE $empty = true,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function outputMemory(
    HH\FIXME\MISSING_PARAM_TYPE $flush = true,
  ): string;
}
