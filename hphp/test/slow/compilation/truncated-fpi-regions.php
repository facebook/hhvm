<?hh

function f($a, $b) :mixed{}

function error() :mixed{
  throw new Exception;
}

function test0($x) :mixed{
  f(reset(42), $x);
}

function test1($x) :mixed{
  f(reset(42), $x ?? 42);
}

function test2($x) :mixed{
  f($x ?? reset(42), $x ?? 42);
}

function test3($x) :mixed{
  if ($x == 24) f(reset(42), $x ?? 42);
  return $x;
}

function test4($x) :mixed{
  f(error(), $x);
}

function test5($x) :mixed{
  f(error(), $x ?? 42);
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
