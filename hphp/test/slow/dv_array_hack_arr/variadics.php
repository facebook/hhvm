<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function variadic($a, ...$b) :mixed{
  echo "================== variadic ======================\n";
  var_dump($b);
  var_dump(is_varray($b));
}

function test($x) :mixed{
  variadic(1, 2, 3, 4);
  variadic(1, 2, ...$x);
  if (count($x) > 0) variadic(...$x);
  if (count($x) > 0) call_user_func_array(variadic<>, $x);
}

<<__EntryPoint>>
function main_variadics() :mixed{
test(dict[]);
test(darray(vec[3, 4, 5]));
test(vec[]);
test(vec[3, 4, 5]);
test(dict['a' => 3, 'b' => 4, 'c' => 5]);
test(vec[3, 4, 5]);
test(Vector{3, 4, 5});
}
