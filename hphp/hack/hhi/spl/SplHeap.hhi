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

abstract class SplHeap<T>
  implements
    Iterator<T>,
    Countable,
    Traversable<T> {

  // Methods
  public function __construct();
  abstract protected function compare(T $value1, T $value2): int;
  public function extract(): T;
  public function insert(T $value): void;
  public function isEmpty(): bool;
  public function recoverFromCorruption(): void;
  public function count(): int;
  public function current(): T;
  public function key(): int;
  public function next(): void;
  public function rewind(): void;
  public function top(): T;
  public function valid(): bool;
}