//// def.php
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

namespace N {
  newtype Foo = int;

  function bless(int $x): Foo {
    return $x;
  }

  function bless2(int $x): \N\Foo {
    return $x;
  }
}

//// use1.php
<?hh

function f(int $x): \N\Foo {
  return \N\bless($x);
}

//// use2.php
<?hh

namespace N {
  function f(int $x): Foo {
    return bless($x);
  }
}
