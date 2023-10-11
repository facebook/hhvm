<?hh
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

type A<Ta,Tb> = dict<Ta, vec<dict<Tb, B<Ta>>>>;
type B<T> = dict<T, vec<T>>;

function expect(dict<int, vec<dict<string, dict<int, vec<int>>>>> $x): void {}

function testit(A<int, string> $a): void {
  expect($a);
}
