<?hh


<<__EntryPoint>>
function main_507() :mixed{
$a = darray['a' => '1', 2 => 2, 'c' => '3'];
var_dump($a);
$a = darray['a' => '1', 2 => 2, 'c' => '3',           'd' => darray['a' => '1', 2 => 2, 'c' => '3']];
var_dump($a);
}
