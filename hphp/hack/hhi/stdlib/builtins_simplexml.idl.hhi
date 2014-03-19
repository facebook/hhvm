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
function simplexml_load_string($data, $class_name = "SimpleXMLElement", $options = 0, $ns = "", $is_prefix = false) { }
function simplexml_load_file($filename, $class_name = "SimpleXMLElement", $options = 0, $ns = "", $is_prefix = false) { }
function libxml_get_errors() { }
function libxml_get_last_error() { }
function libxml_clear_errors() { }
function libxml_use_internal_errors($use_errors = null_variant) { }
function libxml_set_streams_context($streams_context) { }
function libxml_disable_entity_loader($disable = true) { }
class SimpleXMLElement {
  public function __construct($data, $options = 0, $data_is_url = false, $ns = "", $is_prefix = false) { }
  public function offsetExists($index) { }
  public function offsetGet($index) { }
  public function offsetSet($index, $newvalue) { }
  public function offsetUnset($index) { }
  public function getIterator() { }
  public function count() { }
  public function xpath($path) { }
  public function registerXPathNamespace($prefix, $ns) { }
  public function asXML($filename = "") { }
  public function getNamespaces($recursive = false) { }
  public function getDocNamespaces($recursive = false) { }
  public function children($ns = "", $is_prefix = false) { }
  public function getName() { }
  public function attributes($ns = "", $is_prefix = false) { }
  public function addChild($qname, $value = null, $ns = null) { }
  public function addAttribute($qname, $value = null, $ns = null) { }
  public function __toString() { }
  public function __get($name) { }
  public function __set($name, $value) { }
  public function __isset($name) { }
  public function __unset($name) { }
}
class LibXMLError {
  public function __construct() { }
}
class SimpleXMLElementIterator {
  public function __construct() { }
  public function current() { }
  public function key() { }
  public function next() { }
  public function rewind() { }
  public function valid() { }
}
