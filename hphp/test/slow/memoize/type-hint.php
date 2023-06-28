<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__Memoize>>
function func1(int $b, ?int $c,
               bool $d, ?bool $e,
               string $e, ?string $f,
               arraykey $g) :mixed{
  echo "func1 called\n";
  return $b;
}

<<__Memoize>>
function func2(int $a, string $b, arraykey $c) :mixed{
  echo "func2 called\n";
  return $c;
}

function test() :mixed{
  var_dump(func1(1, null, true, null, 'abc', null, 456));
  var_dump(func1(1, null, true, null, 'abc', null, 456));

  var_dump(func1(2, 789, true, false, 'abc', 'def', 'foobaz'));
  var_dump(func1(2, 789, true, false, 'abc', 'def', 'foobaz'));

  var_dump(func2(10, 'b', 3));
  var_dump(func2(10, 'b', 3));

  var_dump(func2(10, 'b', 'z'));
  var_dump(func2(10, 'b', 'z'));
}

<<__EntryPoint>>
function main_type_hint() :mixed{
test();
}
