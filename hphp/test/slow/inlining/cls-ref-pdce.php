<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

final class Bar {
  private static $member = 123;
  public static function set(int $x) {
    self::$member = $x;
  }
}

final class Foo {
  public static function set(int $x) {
    Bar::set($x);
  }
}

function main($x) {
  $a1 = $x;
  $a2 = $x;
  $a3 = $x;
  $a4 = $x;
  $a5 = $x;
  $a6 = $x;
  $a7 = $x;
  $a8 = $x;
  $a9 = $x;
  $a10 = $x;
  $a11 = $x;
  $a12 = $x;
  $a13 = $x;
  $a14 = $x;
  $a15 = $x;
  $a16 = $x;
  $a17 = $x;
  $a18 = $x;
  Foo::set(456);
}

main(1);
main(1);
main(1);
main(1);
