<?hh

function f(inout $x) :mixed{
  return ++$x;
}


<<__EntryPoint>>
function main_double_array_get() :mixed{
$a = varray[0, 1, 2];
$b = varray[1, 2, 3];
var_dump(f(inout $b[$a[1]]));
}
