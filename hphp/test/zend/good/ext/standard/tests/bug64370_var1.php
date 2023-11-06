<?hh <<__EntryPoint>> function main(): void {
echo "\$_SERVER['REQUEST_TIME_NS']: {$_SERVER['REQUEST_TIME_NS']}\n";
echo "time(): " . time() . "\n";
echo "microtime(true): " . (string)(microtime(true)) . "\n";
$d = (clock_gettime_ns(CLOCK_REALTIME)-$_SERVER['REQUEST_TIME_NS'])/1000000;
$d__str = (string)($d);
echo "created in $d__str ms\n";
echo (string)((bool)($d >= 0)) . "\n";
echo "===DONE===\n";
}
