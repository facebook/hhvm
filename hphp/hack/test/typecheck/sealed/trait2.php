<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__Sealed(B::class)>>
interface A {}

trait T {
  require implements A;
}

final class B implements A {
  use T;
}
final class C implements A {
  use T;
}
