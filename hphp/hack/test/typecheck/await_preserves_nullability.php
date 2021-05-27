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

class Foo<T as arraykey> {
  public async function print_if_possible(
    ?Awaitable<T> $blah,
  ): Awaitable<void> {
    $x = await $blah;
    $this->doer($x);
  }

  private function doer(T $item): void {
    if ($item) {
      print $item;
    }
  }
}
