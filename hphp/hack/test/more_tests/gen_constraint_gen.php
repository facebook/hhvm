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

function lol_wat<T1, T2 as T1>(T1 $arg1, T2 $arg2): void {
  return lol_wat($arg1, $arg2);
}
