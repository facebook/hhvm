<?hh


<<__EntryPoint>>
function main_507() :mixed{
$a = dict['a' => '1', 2 => 2, 'c' => '3'];
var_dump($a);
$a = dict['a' => '1', 2 => 2, 'c' => '3',           'd' => dict['a' => '1', 2 => 2, 'c' => '3']];
var_dump($a);
}
