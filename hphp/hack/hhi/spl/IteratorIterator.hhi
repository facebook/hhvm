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

class IteratorIterator<Tv> implements OuterIterator<Tv> {

  // Methods
  public function __construct(Traversable<Tv> $iterator);
  public function getInnerIterator(): Iterator<Tv>;
  public function valid();
  public function key();
  public function current();
  public function next();
  public function rewind();
  public function __call($func, $params);
  protected function _fetch($check);
  protected function _getPosition();
  protected function _setPosition($position);

}
