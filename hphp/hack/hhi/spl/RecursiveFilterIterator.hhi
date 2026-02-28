<?hh /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

abstract class RecursiveFilterIterator<Tv>
  extends FilterIterator<Tv>
  implements
    OuterIterator<~Tv>,
    RecursiveIterator<~Tv> {

  // Methods
  public function __construct(RecursiveIterator<Tv> $iterator);
  public function getChildren(): this;
  public function hasChildren(): bool;

}
