<?hh

function foo ((function (int) : int) $f) : int {
  $x = $f(0);
  return $x;
}

function bar ((function (...) : int) $f) : int {
  return $f(1, 2, 3);
}

function baz ((function (inout string, int) : void) $f) : string {
  $y = 'zzz';
  $f(inout $y, 42);
  return $y;
}
