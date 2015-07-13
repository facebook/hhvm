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

final class EmptyIterator<T> implements Iterator<T> {

  /* current() technically returns ?Tv, but most places that use Iterators
   * check valid() before calling current(), so typing it as non-nullable
   * is what most callsites would expect.*/
  public function current(): T;
  public function key<Tk>(): Tk;
  public function next(): void;
  public function rewind(): void;
  public function valid(): bool;

}
