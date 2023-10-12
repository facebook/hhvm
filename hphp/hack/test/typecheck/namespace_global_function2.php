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

function f(): void {}

//// local.php
<?hh // strict

namespace N {
  function g(): void {
    f();
  }
}
