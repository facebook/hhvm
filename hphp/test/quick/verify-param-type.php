<?hh
// Copyright 2004-2015 Facebook. All Rights Reserved.

// Make sure we're tolerant of code that swallows typehint failures.
function error_handler(...$args) :mixed{
  var_dump($args);
  return true;
}

function test_soft(<<__Soft>> int $foo) :mixed{
  echo "in test_soft\n";
  var_dump(is_int($foo));
}

function test_hard(int $foo) :mixed{
  echo "in test_hard (unreachable)\n";
}

<<__EntryPoint>>
function main() :mixed{
  set_error_handler(error_handler<>);
  test_soft(0.0);
  test_hard(0.0);
  echo "done with main (unreachable)\n";
}
