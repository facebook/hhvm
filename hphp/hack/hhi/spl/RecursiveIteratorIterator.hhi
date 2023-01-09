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
  const int LEAVES_ONLY;
  const int SELF_FIRST;
  const int CHILD_FIRST;
  const int CATCH_GET_CHILD;
  const int STATE_NEXT;
  const int STATE_TEST;
  const int STATE_SELF;
  const int STATE_CHILD;
  const int STATE_START;
  const int NEXT_COMPLETE;
  const int NEXT_REPEAT;

  // Methods
  public function __construct(
    Traversable<Tv> $iterator,
    HH\FIXME\MISSING_PARAM_TYPE $mode = RecursiveIteratorIterator::LEAVES_ONLY,
    HH\FIXME\MISSING_PARAM_TYPE $flags = 0,
  );
  public function getInnerIterator(): Iterator<Tv>;
  public function current();
  public function key();
  public function next(): void;
  public function rewind(): void;
  public function valid(): bool;
  public function beginChildren();
  public function beginIteration();
  public function callGetChildren();
  public function callHasChildren();
  public function endChildren();
  public function endIteration();
  public function getDepth();
  public function getMaxDepth();
  public function getSubIterator(HH\FIXME\MISSING_PARAM_TYPE $level = null);
  public function nextElement();
  public function setMaxDepth(HH\FIXME\MISSING_PARAM_TYPE $max_depth = -1);
  public function call__(
    HH\FIXME\MISSING_PARAM_TYPE $func,
    HH\FIXME\MISSING_PARAM_TYPE $params,
  );

}
