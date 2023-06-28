<?hh


<<__EntryPoint>>
function main_1552() :mixed{
$a = varray[1.23456789e+34, 1E666, 1E666/1E666];
$e = json_encode($a);
var_dump($a);
}
