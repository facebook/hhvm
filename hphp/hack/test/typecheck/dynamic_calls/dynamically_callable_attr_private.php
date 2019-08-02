<?hh // strict
/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

class X {
  <<__DynamicallyCallable>>
  public function pub(): void {}
  <<__DynamicallyCallable>>
  private function priv(): void {}
}
