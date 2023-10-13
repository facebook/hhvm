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

interface A {
  public function render(): string;
}

function foo<T as A>(T $x): void {
  $x->render();
}

function test(): void {
  foo('test');
}
