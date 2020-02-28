<?hh


<<__EntryPoint>>
function main_237() {
$array = darray['1' => darray[2 => 'test']];
unset($array['1'][2]);
var_dump($array['1']);
}
