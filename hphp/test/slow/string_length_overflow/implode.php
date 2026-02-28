<?hh


<<__EntryPoint>>
function main_implode() :mixed{
$stringLarge = str_repeat('*', 300289);
$arrayLarge = array_fill(0, 49981, '*');
$string_implode_2 = implode($stringLarge, $arrayLarge);
}
