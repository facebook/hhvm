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

function finally_basic(): int {
  $a = 23;
  try {
    $a = 456;
  } catch (NonTerminalException $e) {
    $a = 28;
  } catch (SomeException $e) {
    return $a = 23;
  } catch (Exception $e) {
    return $a = 456;
  } finally {
    echo $a;
    $b = 'quack'; // should be fine (!)
    echo $b;
  }
  return $a;
}

function do_something(): void {
  throw new Exception('something like throwing :)');
}

class TestInit {
  private int $foo;

  public function __construct() {
    try {
      do_something();
      $this->foo = 26;
    } finally {
      $this->foo = 25;
    }
  }
}
