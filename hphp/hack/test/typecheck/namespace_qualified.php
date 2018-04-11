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

namespace NS1\SubNS\SubSubNS {
  class C {}
}

namespace NS1 {
  function f(): void {
    new SubNS\SubSubNS\C();
  }
}

namespace NS1\SubNS {
  function f(): void {
    new SubSubNS\C();
  }
}
