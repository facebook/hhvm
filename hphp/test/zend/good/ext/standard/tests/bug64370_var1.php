<?hh <<__EntryPoint>> function main(): void {
echo "\$_SERVER['REQUEST_TIME']: {$_SERVER['REQUEST_TIME']}\n";
$temp_str_39908 = (string)($_SERVER['REQUEST_TIME_FLOAT']);
echo "\$_SERVER['REQUEST_TIME_FLOAT']: {$temp_str_39908}\n";
echo "time(): " . time() . "\n";
echo "microtime(true): " . (string)(microtime(true)) . "\n";
$d = (microtime(true)-$_SERVER['REQUEST_TIME_FLOAT'])*1000;
$d__str = (string)($d);
echo "created in $d__str ms\n";
echo (string)((bool)($d >= 0)) . "\n";
echo "===DONE===\n";
}
