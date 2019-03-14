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

class Super {}

trait Trait1 {
  require extends Super;
}

trait Trait2 {
  use Trait1;
}

abstract class C1 {
  use Trait1;
}
