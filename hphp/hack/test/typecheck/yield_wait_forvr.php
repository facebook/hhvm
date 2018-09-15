<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function show<T>(T $x): void {}

async function gen_int(): Awaitable<int> {
  return 42;
}

async function test(): Awaitable<void> {
  $a = array(
    'foo' => gen_int(),
    'bar' => array(
      'baz' => gen_int(),
      'quux' => array('innermost' => gen_int()),
    ),
    'something else' => array('one last thing' => gen_int()),
  );

  await gen_array_rec($a);
}
