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

interface I {
}

class B {
  public static int $x = 0;
}

class A extends B implements I {
}

class C {
}

function main(): void {
  $x = new A();
  if ($x is B && $x is B && $x is I &&
      !($x is C)) {
    echo 'OK';
  }
  else {
    echo 'FAILURE: test_instanceof.1';
  }
}
