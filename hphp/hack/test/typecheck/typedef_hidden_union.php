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

type A<T> = vec<?T>;

function expect(vec<?int> $x): void { }

function testit(A<?int> $x): void {
  expect($x);
}
