<?hh

<<__EntryPoint>>
function main_hack() {
$x = 'crc32';
var_dump($x('123'));
var_dump(crc32('123'));
}
