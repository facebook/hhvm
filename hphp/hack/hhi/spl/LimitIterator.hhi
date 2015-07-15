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

class LimitIterator<Tv> extends IteratorIterator<Tv> {

  // Methods
  public function __construct(
    Iterator $iterator,
    int $offset = 0,
    int $count = -1,
  );
  public function getPosition();
  public function rewind();
  public function next();
  public function seek($position);
  public function valid();

}
