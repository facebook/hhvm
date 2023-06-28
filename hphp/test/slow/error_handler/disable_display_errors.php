<?hh


<<__EntryPoint>>
function main_disable_display_errors() :mixed{
ini_set('display_errors', 1);
var_dump(ini_get('display_errors'));
ini_set('display_errors', 0);
var_dump(ini_get('display_errors'));
}
