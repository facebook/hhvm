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

class AppendIterator<Tv> extends IteratorIterator<Tv> {

  // Methods
  public function __construct();
  public function append(Iterator<Tv> $it): void;
  public function getArrayIterator(): ArrayIterator<Iterator<Tv>>;
  public function getIteratorIndex(): int;
  public function getInnerIterator(): Iterator<Tv>;
  public function __call($func, $params);
}
