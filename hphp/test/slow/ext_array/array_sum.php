<?hh


<<__EntryPoint>>
function main_array_sum() :mixed{
$a = vec[2, 4, 6, 8];
var_dump(array_sum($a));
$b = dict["a" => 1.2, "b" => 2.3, "c" => 3.4];
var_dump(array_sum($b));
$c = Vector { 3, 6, 9, 12 };
var_dump(array_sum($c));
$d = Set { 4, 8, 16 };
var_dump(array_sum($d));
$e = ImmMap { 'one' => 1, 'two' => 2 };
var_dump(array_sum($e));
$f = Vector{1.2, 3.4};
var_dump(array_sum($f));
}
