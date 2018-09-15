<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

<<__Sealed(B::class)>>
interface A {}

trait T implements A {}

final class B {
  use T;
}
final class C {
  use T;
}
