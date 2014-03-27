<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

class Preparable<T> implements Awaitable<T> {}
class MyPreparable extends Preparable<MyPreparable> {}

async function foo_one(): Awaitable<MyPreparable> {
  $my_preparable = await (new MyPreparable());
  return $my_preparable;
}
/*
function foo_two(): Awaitable<array<MyPreparable>> {
  $my_preparables = yield wait_forv(array(new MyPreparable()));
  yield result($my_preparables);
}
 */
async function foo_three(): Awaitable<(MyPreparable, MyPreparable)> {
  $my_preparables = await genva(new MyPreparable(), new MyPreparable());
  return $my_preparables;
}
