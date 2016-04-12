<?hh

$a = array(2, 4, 6, 8);
var_dump(array_product($a));
var_dump(array_product(array()));
$b = Vector { 3, 6, 9, 12 };
var_dump(array_product($b));
$c = Set { 4, 8, 16 };
var_dump(array_product($c));
$d = ImmMap { 'one' => 1, 'two' => 2 };
var_dump(array_product($d));
$e = Vector{1.2, 3.4};
var_dump(array_product($e));
