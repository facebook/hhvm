<?hh


<<__EntryPoint>>
function main_518() :mixed{
$x = dict['x' => 'y'];
$a = dict['a1' => $x, 'a2' => $x];
$b = dict['a1' => vec[], 'a2' => vec[1,2]];
var_dump(array_merge_recursive($a, $b));
}
