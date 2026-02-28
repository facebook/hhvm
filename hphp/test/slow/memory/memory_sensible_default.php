<?hh


<<__EntryPoint>>
function main_memory_sensible_default() :mixed{
$a = ini_get("memory_limit");
var_dump($a != "9223372036854775807");
}
