<?hh /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

class IteratorIterator<Tv>
  implements OuterIterator<HH\FIXME\POISON_MARKER<Tv>> {

  // Methods
  public function __construct(Traversable<Tv> $iterator);
  public function getInnerIterator(): Iterator<Tv>;
  public function valid(): bool;
  public function key();
  public function current(): HH\FIXME\POISON_MARKER<Tv>;
  public function next(): void;
  public function rewind(): void;
  public function call__(
    HH\FIXME\MISSING_PARAM_TYPE $func,
    HH\FIXME\MISSING_PARAM_TYPE $params,
  );
  protected function _fetch(HH\FIXME\MISSING_PARAM_TYPE $check);
  protected function _getPosition();
  protected function _setPosition(HH\FIXME\MISSING_PARAM_TYPE $position);

}
