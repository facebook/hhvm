<?hh // decl
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/**
 * This file provides type information for some of PHP's predefined interfaces
 *
 * YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!
 */

class ArrayIterator<T> implements KeyedIterator<arraykey, T>,
                                  KeyedTraversable<arraykey, T>,
                                  ArrayAccess<arraykey, T>,
                                  SeekableIterator<T>,
                                  Countable
                                  /* Serializable - not implemented */ {
  public function __construct (mixed $array);
  public function append(mixed $value): void;
  public function asort(): void;
  public function count(): int;
  public function current(): T;
  public function getArrayCopy(): array;
  public function getFlags(): void;
  public function key(): arraykey;
  public function ksort(): void;
  public function natcasesort(): void;
  public function natsort(): void;
  public function next(): void;
  public function offsetExists(arraykey $index): bool;
  public function offsetGet(arraykey $index): T;
  public function offsetSet(arraykey $index, T $newval): void;
  public function offsetUnset(arraykey $index): void;
  public function rewind(): void;
  public function seek(int $position): void;
  public function setFlags(string $flags): void;
  public function uasort(string $cmp_function): void;
  public function uksort(string $cmp_function): void;
  public function valid(): bool;
}
