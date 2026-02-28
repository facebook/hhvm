<?hh


<<__EntryPoint>>
function main_517() :mixed{
$x = dict['x' => 'y'];
$a = dict['a1' => $x, 'a2' => $x];
$b = dict['a1' => vec[1,2,3], 'a2' => vec[1,2,3]];
var_dump(array_merge_recursive($a, $b));
}
