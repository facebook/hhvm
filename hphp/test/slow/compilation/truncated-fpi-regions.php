<?hh

function f($a, $b) {}

function error() {
  throw new Exception;
}

function test0($x) {
  f(reset(42), $x);
}

function test1($x) {
  f(reset(42), $x ?? 42);
}

function test2($x) {
  f($x ?? reset(42), $x ?? 42);
}

function test3($x) {
  if ($x == 24) f(reset(42), $x ?? 42);
  return $x;
}

function test4($x) {
  f(error(), $x);
}

function test5($x) {
  f(error(), $x ?? 42);
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
