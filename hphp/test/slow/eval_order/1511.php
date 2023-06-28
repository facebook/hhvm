<?hh


<<__EntryPoint>>
function main_1511() :mixed{
$a = varray[varray[$id = 1, $id], varray[$id = 2, $id]];
var_dump($a);
$a = varray[+($id = 1), $id, -($id = 2), $id,            !($id = 3), $id, ~($id = 4), $id,            isset($a[$id = 5]), $id];
var_dump($a);
}
