//// N.php
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

namespace N {
  class C {}
}

//// test.php
<?hh

function f(N\C $x): \N\C {
  return $x;
}
