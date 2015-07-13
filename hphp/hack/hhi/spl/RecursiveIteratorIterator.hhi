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

class RecursiveIteratorIterator<Tv> implements OuterIterator<Tv> {

  // Constants
  const LEAVES_ONLY = 0;
  const SELF_FIRST = 1;
  const CHILD_FIRST = 2;
  const CATCH_GET_CHILD = 16;
  const STATE_NEXT = 10;
  const STATE_TEST = 11;
  const STATE_SELF = 12;
  const STATE_CHILD = 13;
  const STATE_START = 14;
  const NEXT_COMPLETE = 10;
  const NEXT_REPEAT = 11;

  // Methods
  public function __construct(
    Traversable<Tv> $iterator,
    $mode = RecursiveIteratorIterator::LEAVES_ONLY,
    $flags = 0,
  );
  public function getInnerIterator(): Iterator<Tv>;
  public function current();
  public function key();
  public function next();
  public function rewind();
  public function valid();
  public function beginChildren();
  public function beginIteration();
  public function callGetChildren();
  public function callHasChildren();
  public function endChildren();
  public function endIteration();
  public function getDepth();
  public function getMaxDepth();
  public function getSubIterator($level = null);
  public function nextElement();
  public function setMaxDepth($max_depth = -1);
  public function __call($func, $params);

}
