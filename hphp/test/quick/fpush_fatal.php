<?hh
abstract final class FpushFatal { public static $x; }
function blah() {}
function foo() {
  $x = FpushFatal::$x;
  blah(blah(), $x());
}
<<__EntryPoint>> function main(): void {
FpushFatal::$x = 'asdasdasd';
foo();
}
