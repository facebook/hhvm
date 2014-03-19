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

<<Foo(1,2,3), Bar>>
class C1 {
}

<<SingleAttribute>>
class C2 {
}

<<SingleAttributeWithOneParam(1)>>
class C3 {
}

<<SingleAttributeWithTwoParams(1,2)>>
class C4 {
}

<<Multiple(1), Attributes(2,3), Foo>>
class C5 {
}
