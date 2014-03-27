<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

interface A {}
interface B {
  public function getInt(): int;
}
interface C<T as B> {
  public function genT(): Awaitable<T>;
}
interface D {
}

async function bar(
    A $rule,
  ): Awaitable<int> {
  invariant($rule instanceof C, 'lala');
  $delegate = await $rule->genT();
  $delegate->getInt();
  return 123;
}

