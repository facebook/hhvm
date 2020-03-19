<?hh /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int LIBXML_COMPACT = 0;
const int LIBXML_DTDATTR = 0;
const int LIBXML_DTDLOAD = 0;
const int LIBXML_DTDVALID = 0;
const int LIBXML_HTML_NOIMPLIED = 0;
const int LIBXML_HTML_NODEFDTD = 0;
const int LIBXML_NOBLANKS = 0;
const int LIBXML_NOCDATA = 0;
const int LIBXML_NOEMPTYTAG = 0;
const int LIBXML_NOENT = 0;
const int LIBXML_NOERROR = 0;
const int LIBXML_NONET = 0;
const int LIBXML_NOWARNING = 0;
const int LIBXML_NOXMLDECL = 0;
const int LIBXML_NSCLEAN = 0;
const int LIBXML_PARSEHUGE = 0;
const int LIBXML_PEDANTIC = 0;
const int LIBXML_XINCLUDE = 0;
const int LIBXML_ERR_ERROR = 0;
const int LIBXML_ERR_FATAL = 0;
const int LIBXML_ERR_NONE = 0;
const int LIBXML_ERR_WARNING = 0;
const int LIBXML_VERSION = 0;
const int LIBXML_SCHEMA_CREATE = 0;

<<__PHPStdLib>>
function libxml_get_errors();
<<__PHPStdLib>>
function libxml_get_last_error();
<<__PHPStdLib>>
function libxml_clear_errors();
<<__PHPStdLib>>
function libxml_use_internal_errors(bool $use_errors = false);
<<__PHPStdLib>>
function libxml_suppress_errors(bool $suppress_errors);
<<__PHPStdLib>>
function libxml_set_streams_context(resource $streams_context);
<<__PHPStdLib>>
function libxml_disable_entity_loader(bool $disable = true);

<<__PHPStdLib>>
function simplexml_load_string(string $data, string $class_name = "SimpleXMLElement", int $options = 0, string $ns = "", bool $is_prefix = false);
<<__PHPStdLib>>
function simplexml_load_file(string $filename, string $class_name = "SimpleXMLElement", int $options = 0, string $ns = "", bool $is_prefix = false);

class SimpleXMLElement {
  public function __construct(string $data, int $options = 0, bool $data_is_url = false, string $ns = "", bool $is_prefix = false);
  public function offsetExists($index);
  public function offsetGet($index);
  public function offsetSet($index, $newvalue);
  public function offsetUnset($index);
  public function getIterator();
  public function count();
  public function xpath(string $path);
  public function registerXPathNamespace(string $prefix, string $ns);
  public function asXML(string $filename = "");
  public function getNamespaces(bool $recursive = false);
  public function getDocNamespaces(bool $recursive = false);
  public function children(string $ns = "", bool $is_prefix = false);
  public function getName();
  public function attributes(string $ns = "", bool $is_prefix = false);
  public function addChild(string $qname, string $value = "", $ns = null);
  public function addAttribute(string $qname, string $value = "", string $ns = "");
  public function __toString();
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
class SimpleXMLElementIterator {
  public function __construct();
  public function current();
  public function key();
  public function next();
  public function rewind();
  public function valid();
}
