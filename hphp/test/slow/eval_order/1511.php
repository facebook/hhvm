<?hh


<<__EntryPoint>>
function main_1511() :mixed{
$id = 1; $e00 = $id; $e01 = $id;
$id = 2; $e10 = $id; $e11 = $id;
$a = vec[vec[$e00, $e01], vec[$e10, $e11]];
var_dump($a);
$id = 1; $f0 = +$id; $f1 = $id;
$id = 2; $f2 = -$id; $f3 = $id;
$id = 3; $f4 = !$id; $f5 = $id;
$id = 4; $f6 = ~$id; $f7 = $id;
$id = 5; $f8 = isset($a[$id]); $f9 = $id;
$a = vec[$f0, $f1, $f2, $f3, $f4, $f5, $f6, $f7, $f8, $f9];
var_dump($a);
}
