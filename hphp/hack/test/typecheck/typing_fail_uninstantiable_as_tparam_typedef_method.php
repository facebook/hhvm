<?hh
/**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

abstract final class Bar {}

type Baz = Bar;  // TODO: why uninstantiability error point to this line?

class C<T as Baz> {
  public function bar<T2>(T2 $u): void {
  }
}
