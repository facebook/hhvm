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

class X {
  const string X1 = 'field1';
  const bool X2 = true;
}

// Should fail for using a non int/string key
type myshape = shape(
  X::X1 => int,
  X::X2 => bool,
);
