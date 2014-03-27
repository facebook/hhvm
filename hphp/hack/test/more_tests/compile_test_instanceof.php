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

interface I {
}

class B {
  static int $x = 0;
}

class A extends B implements I {
}

class C {
}

function main(): void {
  $x = new A();
  if ($x instanceof B && $x instanceof B && $x instanceof I &&
      !($x instanceof C)) {
    echo 'OK';
  }
  else {
    echo 'FAILURE: test_instanceof.1';
  }
}
