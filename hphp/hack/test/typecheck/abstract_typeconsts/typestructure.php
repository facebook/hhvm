<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class A {
  abstract const type T as num = int;
}

class B extends A {}

function f(): void {
  type_structure(A::class, 'T'); // error
  type_structure(B::class, 'T');
}
