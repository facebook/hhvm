<?hh

function f(inout $x) :mixed{
  return ++$x;
}


<<__EntryPoint>>
function main_double_array_get() :mixed{
$a = vec[0, 1, 2];
$b = vec[1, 2, 3];
var_dump(f(inout $b[$a[1]]));
}
