<?hh
// Copyright 2004-2015 Facebook. All Rights Reserved.

// Make sure we're tolerant of code that swallows typehint failures.
function error_handler(...$args) {
  var_dump($args);
  return true;
}

function test_soft(@int $foo) {
  echo "in test_soft\n";
  var_dump(is_int($foo));
}

function test_hard(int $foo) {
  echo "in test_hard (unreachable)\n";
}

<<__EntryPoint>>
function main() {
  set_error_handler(fun('error_handler'));
  test_soft(0.0);
  test_hard(0.0);
  echo "done with main (unreachable)\n";
}
