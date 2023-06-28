<?hh


<<__EntryPoint>>
function main_519() :mixed{
$x = darray['x' => 'y'];
$a = darray['a1' => $x, 'a2' => $x];
$b = darray['a1' => varray[1,2], 'a2' => varray[3,4]];
var_dump(array_merge_recursive($a, $b));
}
