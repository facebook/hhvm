<?hh // strict
/**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

<<Foo(1,2,3), Bar>>
type T1 = int;

<<SingleAttribute>>
type T2 = ?string;

<<SingleAttributeWithOneParam(1)>>
newtype T3 as int = int;

<<SingleAttributeWithTwoParams(1,2)>>
type T4 = array<int>;

<<Multiple(1), Attributes(2,3), Foo>>
type T5 = (function(int): bool);
