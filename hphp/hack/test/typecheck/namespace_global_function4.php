//// global.php
<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function f(): int {
  return 42;
}

//// local.php
<?hh // strict

namespace N {
  function f(): string {
    return 'this one should be used';
  }

  function g(): string {
    return f();
  }
}
