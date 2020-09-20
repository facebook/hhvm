<?hh // strict

<<__EntryPoint>>
function main_hack_strict() {
$x = 'crc32';
var_dump($x('123'));
var_dump(crc32('123'));
}
