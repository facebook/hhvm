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

abstract class RecursiveFilterIterator<Tv> extends FilterIterator<Tv>
  implements
    OuterIterator<Tv>,
    RecursiveIterator<Tv> {

  // Methods
  public function __construct(RecursiveIterator<Tv> $iterator);
  public function getChildren();
  public function hasChildren();

}
