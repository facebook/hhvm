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

function finally_typing1(): string {
  $a = 23;
  try {
    do_something();
  } finally {
    $a = 'string'; // this definition escapes the clause
  }
  return $a;
}

function finally_typing2(): string {
  $a = 23;
  try {
    do_something();
    return 'string';
  } finally {
    // this definition escapes the clause, even with terminality
    $a = 'string';
  }
  return $a;
}

// with a different story with respect to unreachable code ...
// function finally_typing3(): int {
//   $a = 23;
//   try {
//     do_something();
//     $a = 25;
//     return $a; // terminal block
//   } finally {
//     // this assignment beats out the original, but it doesn't matter
//     // because the try is fully terminal
//     $a = 'string';
//   }
//   return $a;
// }
