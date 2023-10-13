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

namespace NS1\SubNS1 {
  class C {}
}

namespace NS2 {
  use NS1 as X;
  use NS1\SubNS1, NS1\SubNS1\C as Y;
  use \NS1\SubNS1 as Z;

  function f(): void {
    new \NS1\SubNS1\C();
    new X\SubNS1\C();
    new SubNS1\C();
    new Y();
    new Z\C();
  }
}
