<?hh /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

abstract class FilterIterator<Tv> extends IteratorIterator<Tv> {

  // Methods
  public function __construct(Iterator<Tv> $it);
  public function rewind(): void;
  public abstract function accept(): bool;
  public function next(): void;
  public function valid(): bool;
  public function key(): HH\FIXME\MISSING_RETURN_TYPE;
  public function current(): ~Tv;
  protected function __clone(): HH\FIXME\MISSING_RETURN_TYPE;
  public function call__(
    HH\FIXME\MISSING_PARAM_TYPE $func,
    HH\FIXME\MISSING_PARAM_TYPE $params,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getInnerIterator(): Iterator<Tv>;
}
