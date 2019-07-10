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

// Reqs still works
/*
interface I<T> where T as num {
  require extends Test<T>;
}
*/

function foo(I<int> $i): Test<int> {
  return $i;
}
/*
class Test2 {}

interface J where this as Test2{
  // require extends Test2;
}

function foo2(J $i): Test2 {
  return $i;
}
*/
