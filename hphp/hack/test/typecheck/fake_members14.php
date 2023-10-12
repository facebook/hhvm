<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public static ?int $fld = 42;
}

function foo((function():void) $f):void {
  C::$fld = null;
  $f();
}
function expect_int(int $x):void { }
<<__EntryPoint>>
function testit():void {
  if (C::$fld !== null) {
    foo(() ==> expect_int(C::$fld));
  }
}
