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

function main(): void {
  $x = 1;
  $y = 'test';
  if("this$x$y" === 'this1test') {
    echo 'OK';
  }
  else {
    echo 'FAILURE: test2.1 ', "this$x$y";
  }
}
