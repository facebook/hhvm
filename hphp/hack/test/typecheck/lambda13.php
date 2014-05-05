<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

function main($fooby): void {
  $capture1 = "yo";
  $get_func = ($x, $y) ==> {
    return $more ==> $x . $y . $capture1 . $more . $fooby;
  };

  $f = $get_func("one ", "two ");
  echo $f("\n");
}
