<?hh // decl /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
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

function libxml_get_errors();
function libxml_get_last_error();
function libxml_clear_errors();
function libxml_use_internal_errors($use_errors = null);
function libxml_set_streams_context($streams_context);
function libxml_disable_entity_loader($disable = true);

function simplexml_load_string($data, $class_name = "SimpleXMLElement", $options = 0, $ns = "", $is_prefix = false);
function simplexml_load_file($filename, $class_name = "SimpleXMLElement", $options = 0, $ns = "", $is_prefix = false);
class SimpleXMLElement {
  public function __construct($data, $options = 0, $data_is_url = false, $ns = "", $is_prefix = false);
  public function offsetExists($index);
  public function offsetGet($index);
  public function offsetSet($index, $newvalue);
  public function offsetUnset($index);
  public function getIterator();
  public function count();
  public function xpath($path);
  public function registerXPathNamespace($prefix, $ns);
  public function asXML($filename = "");
  public function getNamespaces($recursive = false);
  public function getDocNamespaces($recursive = false);
  public function children($ns = "", $is_prefix = false);
  public function getName();
  public function attributes($ns = "", $is_prefix = false);
  public function addChild($qname, $value = null, $ns = null);
  public function addAttribute($qname, $value = null, $ns = null);
  public function __toString();
  public function __get($name);
  public function __set($name, $value);
  public function __isset($name);
  public function __unset($name);
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
