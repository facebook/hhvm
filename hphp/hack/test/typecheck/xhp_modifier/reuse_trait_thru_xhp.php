<?hh // strict
/**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

trait A {}
class Base {
  use A;
}

xhp class foo:core {
  use A;
}

xhp class foo extends Base {
  attribute :foo:core;
}
