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

abstract class C {
  public async function gen(): Awaitable<void> {}

  public function f(): void {
    foreach (vec[] as $_) {
    }
    $this->gen();
  }
}
