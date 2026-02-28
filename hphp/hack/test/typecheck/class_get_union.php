//// file1.php
<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public static function test(bool $b, int $x): void {
    $c = $b ? C1::class : C2::class;
    $c::f($x);
  }
}

//// file2.php
<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C1<T> {
  public static function f(T $_): void {}
}

abstract class C2 extends C1<this::T> {
  abstract const type T;

  public static function f(this::T $_): void {}
}
