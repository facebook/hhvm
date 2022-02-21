<?hh /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

class RecursiveIteratorIterator<Tv> implements OuterIterator<Tv> {

  // Constants
  const int LEAVES_ONLY = 0;
  const int SELF_FIRST = 1;
  const int CHILD_FIRST = 2;
  const int CATCH_GET_CHILD = 16;
  const int STATE_NEXT = 10;
  const int STATE_TEST = 11;
  const int STATE_SELF = 12;
  const int STATE_CHILD = 13;
  const int STATE_START = 14;
  const int NEXT_COMPLETE = 10;
  const int NEXT_REPEAT = 11;

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
  public function call__($func, $params);

}
