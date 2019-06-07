<?hh


<<__EntryPoint>>
function main_microtime() {
$time_start = microtime(true);
var_dump($time_start > 0);
}
