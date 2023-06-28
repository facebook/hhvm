<?hh


<<__EntryPoint>>
function main_cast_float_to_string() :mixed{
$negzero = 0.0 * -1;
var_dump($negzero);
var_dump((string)$negzero);
}
