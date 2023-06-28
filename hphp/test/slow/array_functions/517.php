<?hh


<<__EntryPoint>>
function main_517() :mixed{
$x = darray['x' => 'y'];
$a = darray['a1' => $x, 'a2' => $x];
$b = darray['a1' => varray[1,2,3], 'a2' => varray[1,2,3]];
var_dump(array_merge_recursive($a, $b));
}
