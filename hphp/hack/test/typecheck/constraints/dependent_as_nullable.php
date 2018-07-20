<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class C {
  abstract const type TArgs as ?shape(...);
  final public static function foo(): this::TArgs {
    //UNSAFE
  }
}

function testit(bool $b): void {
  $cb = C::class;
  $x = $cb::foo();
  if ($x === null)
    return;
  $y = Shapes::toDict($x);
}
