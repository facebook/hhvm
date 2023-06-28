<?hh


<<__EntryPoint>>
function main_237() :mixed{
$array = darray['1' => darray[2 => 'test']];
unset($array['1'][2]);
var_dump($array['1']);
}
