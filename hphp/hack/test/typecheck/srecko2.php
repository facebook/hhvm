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

interface SomeInterface { }
class Foo { }

function f(Foo $bar): Foo {
  if ($bar is SomeInterface) {
    // At this point, $bar is of SomeInterface type.
    // All typing from Foo is lost.
  }
  return $bar;
}
