<?hh

function f($x, $y) :mixed{
  return $x[0][$y++] ?: false;
}

<<__EntryPoint>>
function main_1738() :mixed{
var_dump(f(vec[vec[0, 1, 2]], 0));
var_dump(f(vec[vec[0, 1, 2]], 1));
}
