<?hh // strict
// Copyright 2019-present Facebook. All Rights Reserved.

final class TestPocketUniversesAtom {
  public static function foo() : void {
    $x = :@foo;
    var_dump($x);
  }
}
