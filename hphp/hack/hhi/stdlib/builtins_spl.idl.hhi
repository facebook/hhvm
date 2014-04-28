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
function spl_classes() { }
function spl_object_hash($obj) { }
function hphp_object_pointer($obj) { }
function hphp_get_this() { }
function class_implements($obj, $autoload = true) { }
function class_parents($obj, $autoload = true) { }
function class_uses($obj, $autoload = true) { }
function iterator_apply($obj, $func, $params = null) { }
function iterator_count($obj) { }
function iterator_to_array($obj, $use_keys = true) { }
function spl_autoload_call($class_name) { }
function spl_autoload_extensions($file_extensions = null) { }
function spl_autoload_functions() { }
function spl_autoload_register($autoload_function = null_variant, $throws = true, $prepend = false) { }
function spl_autoload_unregister($autoload_function) { }
function spl_autoload($class_name, $file_extensions = null) { }

class SplDoublyLinkedList<T> implements Iterator<T>, ArrayAccess<int, T>, Countable {
  public function bottom(): T { throw new Exception(); }
  public function isEmpty(): bool { throw new Exception(); }
  public function key(): int { throw new Exception(); }
  public function pop(): T { throw new Exception(); }
  public function prev(): void {}
  public function push(T $val): void {}
  public function serialize(): string { throw new Exception(); }
  public function shift(): T { throw new Exception(); }
  public function top(): T { throw new Exception(); }
  public function unserialize(string $str): void {}
  public function unshift(T $val): void {}

  public function current(): T { throw new Exception(); }
  public function next(): void {}
  public function rewind(): void {}
  public function valid(): bool { throw new Exception(); }

  public function offsetExists(/*int*/ $key): bool { throw new Exception(); }
  public function offsetGet(/*int*/ $key): T { throw new Exception(); }
  public function offsetSet(/*int*/ $key, T $val): this { throw new Exception(); }
  public function offsetUnset(/*int*/ $key): this { throw new Exception(); }

  public function count(): int { throw new Exception(); }
}

class SplQueue<T> extends SplDoublyLinkedList<T> {
  public function __construct() {}
  public function dequeue(): T { throw new Exception(); }
  public function enqueue(T $val): void {}
}
