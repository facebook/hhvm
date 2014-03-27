<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

function f1(): Vector<string> {
  return Vector {'hello', 'world'};
}

function f2(): Vector<int> {
  return Vector {};
}

function f3(): Vector<Vector<string>> {
  return Vector {f1(), Vector {'a', 'b', 'c'}, Vector {}};
}
