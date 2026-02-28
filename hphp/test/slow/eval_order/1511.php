<?hh


<<__EntryPoint>>
function main_1511() :mixed{
$a = vec[vec[$id = 1, $id], vec[$id = 2, $id]];
var_dump($a);
$a = vec[+($id = 1), $id, -($id = 2), $id,            !($id = 3), $id, ~($id = 4), $id,            isset($a[$id = 5]), $id];
var_dump($a);
}
