<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function f(int ...$x) {
  foreach ($x as $y) {
    var_dump($y);
  }
}

function g() : varray<int> {
  return varray[1,2,3,4];
}


<<__EntryPoint>>
function main_argument_as_function_call() {
$h = fun("g");

f(...$h());
}
