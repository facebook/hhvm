<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */
function f(): void {}

function foo(): int {
  $a = 23;
  try {
    $a = 456;
    f();
  } catch (MyException $e) {
    // Now that this is a non-terminal, this is bad
    $a = 'duck';
  } catch (Exception $e) {
    $a = 123;
  }
  return $a;
}

class MyException extends Exception {}
