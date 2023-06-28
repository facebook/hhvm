<?hh
abstract final class FpushFatal { public static $x; }
function blah() :mixed{}
function foo() :mixed{
  $x = FpushFatal::$x;
  blah(blah(), $x());
}
<<__EntryPoint>> function main(): void {
FpushFatal::$x = 'asdasdasd';
foo();
}
