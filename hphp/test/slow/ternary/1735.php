<?hh

function foo($a) :mixed{
  $x = $a ? 1 : 0;
  return $x - 5;
}

<<__EntryPoint>>
function main_1735() :mixed{
var_dump(foo(1, 2, 3));
var_dump(foo(0, 2, 3));
}
