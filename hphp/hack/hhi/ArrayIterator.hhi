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

class ArrayIterator<T> implements KeyedIterator<int, T>,
                                  KeyedTraversable<int, T>,
                                  ArrayAccess<int, T>,
                                  SeekableIterator,
                                  Countable,
                                  Serializable {
  public function __construct (mixed $array);
  public function append(mixed $value): void;
  public function asort(): void;
  public function count(): int;
  public function current(): mixed;
  public function getArrayCopy(): array;
  public function getFlags(): void;
  public function key(): mixed;
  public function ksort(): void;
  public function natcasesort(): void;
  public function natsort(): void;
  public function next(): void;
  public function offsetExists(string $index): void;
  public function offsetGet(string $index): mixed;
  public function offsetUnset(string $index): void;
  public function rewind(): void;
  public function seek(int $position): void;
  public function serialize(): string;
  public function setFlags(string $flags): void;
  public function uasort(string $cmp_function): void;
  public function uksort(string $cmp_function): void;
  public function unserialize(string $serialized): string;
  public function valid(): bool;
}
