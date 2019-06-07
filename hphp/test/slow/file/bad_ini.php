<?hh


<<__EntryPoint>>
function main_bad_ini() {
$foo = parse_ini_string("hello sailor");
var_dump($foo);
}
