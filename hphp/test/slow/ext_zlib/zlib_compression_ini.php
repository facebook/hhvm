<?hh


// Using old values because in PHP 5.x as soon as the first output happens
// via var_dump, it throws a warning:
// PHP Warning:
//    ini_set(): Cannot change zlib.output_compression - headers already sent
// on the next zlib.output_compression ini_set() call. So avoiding that if
// we test with PHP 5.x
<<__EntryPoint>>
function main_zlib_compression_ini() :mixed{
$old1 = ini_set('zlib.output_compression', 'on');
$old2 = ini_set('zlib.output_compression_level', '9');
$old3 = ini_set('zlib.output_compression', 'off');
$old4 = ini_set('zlib.output_compression_level', '4');
var_dump($old1);
var_dump($old2);
var_dump($old3);
var_dump($old4);
}
