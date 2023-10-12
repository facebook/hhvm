<?hh // strict

class C {
  public static ?int $thing = null;
}
function expect_int(int $x):void { }

<<__EntryPoint>>
function test_flow():void {
  do {
    break;
  } while (C::$thing !== null);
  expect_int(C::$thing);
}
