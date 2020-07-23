<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

/**
 * This file provides type information for some of PHP's predefined interfaces
 *
 * YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!
 */

class ArrayIterator<T> implements KeyedIterator<arraykey, T>,
                                  KeyedTraversable<arraykey, T>
                                  /* Serializable - not implemented */ {
  public function __construct (mixed $array);
  public function current(): T;
  public function getFlags(): void;
  public function key(): arraykey;
  public function next(): void;
  public function rewind(): void;
  public function setFlags(string $flags): void;
  public function valid(): bool;
}
