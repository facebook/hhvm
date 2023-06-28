<?hh


<<__EntryPoint>>
function main_microtime() :mixed{
$time_start = microtime(true);
var_dump($time_start > 0);
}
