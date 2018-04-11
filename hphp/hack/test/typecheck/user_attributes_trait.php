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

<<Foo(1,2,3), Bar>> trait T1 {}

<<SingleAttribute>> trait T2 {}

<<SingleAttributeWithOneParam(1)>> trait T3 {}

<<SingleAttributeWithTwoParams(1,2)>> trait T4 {}

<<Multiple(1), Attributes(2,3), Foo>> trait T5 {}
