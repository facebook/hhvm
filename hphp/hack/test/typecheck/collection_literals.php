<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */
function f1() : Vector<int> {
  return Vector {};
}
function f2() : Vector<int> {
  return Vector { 1, 2 };
}
function f3() : Map<string,int> {
  return Map {};
}
function f4() : Map<int,string> {
  return Map { 1 => 'a' };
}
