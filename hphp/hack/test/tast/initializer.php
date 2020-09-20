<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

class A {}
class B {}

abstract final class C {
  private static $d = darray[
    'a' => A::class,
    'b' => B::class,
  ];
  private static $s = Set {'foo'};
}
