<?hh
abstract final class FpushFatal { public static $x; }
function blah() {}
function foo() {
  $x = FpushFatal::$x;
  blah(blah(), $x());
}

FpushFatal::$x = 'asdasdasd';
foo();
