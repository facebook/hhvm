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

async function foo<Tu as B>(C<Tu> $x):Awaitable<void> {
  $d = await $x->genT();
}

async function bar<Tu as B>(
    A $rule,
    C<Tu> $rule2,
  ): Awaitable<int> {
  if (!($rule instanceof C)) {
    echo 'blah';
    return 123;
  }
//  hh_show($rule);
  $delegate = await $rule->genT();
//  hh_show($delegate);
  $delegate->getInt();
  return 123;
}
