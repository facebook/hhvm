<?hh


<<__EntryPoint>>
function main_parse_ini_string_putenv() {
var_dump(getenv('herp'));
putenv('herp=derp');
var_dump(parse_ini_string('foo=${herp}'));
}
