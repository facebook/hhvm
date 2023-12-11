<?hh


<<__EntryPoint>>
function main_237() :mixed{
$array = dict['1' => dict[2 => 'test']];
unset($array['1'][2]);
var_dump($array['1']);
}
