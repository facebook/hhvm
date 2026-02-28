<?hh

function f($x, $y) :mixed{
  $z = 32;
  return $x && $y ?: $z;
}

<<__EntryPoint>>
function main_1847() :mixed{
var_dump(f(false, false));
var_dump(f(true, true));
}
