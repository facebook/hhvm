<?hh


<<__EntryPoint>>
function main_518() :mixed{
$x = darray['x' => 'y'];
$a = darray['a1' => $x, 'a2' => $x];
$b = darray['a1' => varray[], 'a2' => varray[1,2]];
var_dump(array_merge_recursive($a, $b));
}
