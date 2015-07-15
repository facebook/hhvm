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

abstract class FilterIterator<Tv> extends IteratorIterator<Tv> {

  // Methods
  public function __construct(Iterator<Tv> $it);
  public function rewind();
  public abstract function accept(): bool;
  public function next();
  public function valid();
  public function key();
  public function current();
  protected function __clone();
  public function getInnerIterator(): Iterator<Tv>;
}
