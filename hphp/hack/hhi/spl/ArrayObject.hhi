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

class ArrayObject
  implements
    IteratorAggregate<mixed>,
    ArrayAccess<string, mixed>,
    Serializable,
    Countable,
    Traversable<mixed> {

  use StrictKeyedIterable<string, mixed>;

  // Constants
  const STD_PROP_LIST = 1;
  const ARRAY_AS_PROPS = 2;

  // Methods
  public function __construct(
    $input = null,
    $flags = null,
    string $iterator_class = "ArrayIterator",
  );
  public function append($value);
  public function asort($sort_flags = 0);
  public function count();
  public function exchangeArray($input);
  public function getArrayCopy();
  public function getFlags();
  public function getIterator();
  public function getIteratorClass();
  public function ksort($sort_flags = 0);
  public function natcasesort();
  public function natsort();
  public function offsetExists($index);
  public function offsetGet($index);
  public function offsetSet($index, $newval);
  public function offsetUnset($index);
  public function serialize();
  public function setFlags(int $flags);
  public function setIteratorClass(string $iterator_class);
  public function uasort($cmp_function);
  public function uksort($cmp_function);
  public function unserialize($serialized);
  public function __set($name, $value);
  public function __get(string $name);
  public function __isset(string $name);
  public function __unset(string $name);
  public function __debugInfo();
  public function lastKey();
  public function lastValue();
  public function firstKey();
  public function firstValue();
  public function toImmSet();
  public function toSet();
  public function concat($iterable);
  public function toImmMap();
  public function filter($callback);
  public function filterWithKey($callback);
  public function toMap();
  public function toImmVector();
  public function toArray();
  public function toVector();
  public function toKeysArray();
  public function map($callback);
  public function values();
  public function skip($n);
  public function keys();
  public function mapWithKey($callback);
  public function lazy();
  public function takeWhile($fn);
  public function toValuesArray();
  public function zip($iterable);
  public function take($n);
  public function slice($start, $len);
  public function skipWhile($fn);

}
