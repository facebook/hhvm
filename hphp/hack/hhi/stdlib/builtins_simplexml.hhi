<?hh /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int LIBXML_COMPACT;
const int LIBXML_DTDATTR;
const int LIBXML_DTDLOAD;
const int LIBXML_DTDVALID;
const int LIBXML_HTML_NOIMPLIED;
const int LIBXML_HTML_NODEFDTD;
const int LIBXML_NOBLANKS;
const int LIBXML_NOCDATA;
const int LIBXML_NOEMPTYTAG;
const int LIBXML_NOENT;
const int LIBXML_NOERROR;
const int LIBXML_NONET;
const int LIBXML_NOWARNING;
const int LIBXML_NOXMLDECL;
const int LIBXML_NSCLEAN;
const int LIBXML_PARSEHUGE;
const int LIBXML_PEDANTIC;
const int LIBXML_XINCLUDE;
const int LIBXML_ERR_ERROR;
const int LIBXML_ERR_FATAL;
const int LIBXML_ERR_NONE;
const int LIBXML_ERR_WARNING;
const int LIBXML_VERSION;
const int LIBXML_SCHEMA_CREATE;

<<__PHPStdLib>>
function libxml_get_errors(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function libxml_get_last_error(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function libxml_clear_errors(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function libxml_use_internal_errors(
  bool $use_errors = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function libxml_suppress_errors(
  bool $suppress_errors,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function libxml_set_streams_context(
  resource $streams_context,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function libxml_disable_entity_loader(
  bool $disable = true,
): HH\FIXME\MISSING_RETURN_TYPE;

<<__PHPStdLib>>
function simplexml_load_string(
  string $data,
  string $class_name = "SimpleXMLElement",
  int $options = 0,
  string $ns = "",
  bool $is_prefix = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function simplexml_load_file(
  string $filename,
  string $class_name = "SimpleXMLElement",
  int $options = 0,
  string $ns = "",
  bool $is_prefix = false,
): HH\FIXME\MISSING_RETURN_TYPE;

class SimpleXMLElement {
  public function __construct(
    string $data,
    int $options = 0,
    bool $data_is_url = false,
    string $ns = "",
    bool $is_prefix = false,
  );
  public function offsetExists(
    HH\FIXME\MISSING_PARAM_TYPE $index,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function offsetGet(
    HH\FIXME\MISSING_PARAM_TYPE $index,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function offsetSet(
    HH\FIXME\MISSING_PARAM_TYPE $index,
    HH\FIXME\MISSING_PARAM_TYPE $newvalue,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function offsetUnset(
    HH\FIXME\MISSING_PARAM_TYPE $index,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getIterator(): HH\FIXME\MISSING_RETURN_TYPE;
  public function count(): HH\FIXME\MISSING_RETURN_TYPE;
  public function xpath(string $path): HH\FIXME\MISSING_RETURN_TYPE;
  public function registerXPathNamespace(
    string $prefix,
    string $ns,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function asXML(string $filename = ""): HH\FIXME\MISSING_RETURN_TYPE;
  public function getNamespaces(
    bool $recursive = false,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getDocNamespaces(
    bool $recursive = false,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function children(
    string $ns = "",
    bool $is_prefix = false,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getName(): HH\FIXME\MISSING_RETURN_TYPE;
  public function attributes(
    string $ns = "",
    bool $is_prefix = false,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function addChild(
    string $qname,
    string $value = "",
    HH\FIXME\MISSING_PARAM_TYPE $ns = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function addAttribute(
    string $qname,
    string $value = "",
    string $ns = "",
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function __toString(): string;
}
class LibXMLError {
  // php.net/manual/en/class.libxmlerror.php
  public function __construct();
  public int $level;
  public int $code;
  public int $column;
  public string $message;
  public string $file;
  public int $line;
}
class SimpleXMLElementIterator /* implements Iterator<SimpleXMLElement> */ {
  public function __construct();
  public function current(): HH\FIXME\MISSING_RETURN_TYPE;
  public function key(): HH\FIXME\MISSING_RETURN_TYPE;
  public function next(): HH\FIXME\MISSING_RETURN_TYPE;
  public function rewind(): HH\FIXME\MISSING_RETURN_TYPE;
  public function valid(): bool;
}
