<?hh


<<__EntryPoint>>
function main_fread_length() :mixed{
$h = fopen(__FILE__, 'r');
var_dump(fread($h, 0));
}
