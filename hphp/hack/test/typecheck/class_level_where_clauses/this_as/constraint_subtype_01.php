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

abstract class Test<T> {}

interface I<T> where this as Test<T>, T as num {}  // I is subtype of Test

function foo<T>(I<T> $i): Test<T> where T as num {
  return $i;
}
