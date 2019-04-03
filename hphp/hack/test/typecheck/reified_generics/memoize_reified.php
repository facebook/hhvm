<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

final class C {
  <<__Memoize>>
  public function f<reify T>(): void {}

  <<__Memoize>>
  public static function fs<reify T>(): void {}
}

<<__Memoize>>
function f<reify T>(): void {}
