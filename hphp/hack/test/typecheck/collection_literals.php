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
function f5() : StableMap<string,StableMap<string,int>> {
  return StableMap {};
}
function f6() : StableMap<string,StableMap<string,int>> {
  return StableMap { 'a' => StableMap { 'a' => 1 } };
}
function f7() : Map<int,StableMap<string,int>> {
  return Map { 1 => StableMap { 'a' => 2 } };
}

