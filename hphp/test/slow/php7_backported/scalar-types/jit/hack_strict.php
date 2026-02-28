<?hh

<<__EntryPoint>>
function main_hack_strict() :mixed{
$x = 'crc32';
var_dump(HH\dynamic_fun($x)('123'));
var_dump(crc32('123'));
}
