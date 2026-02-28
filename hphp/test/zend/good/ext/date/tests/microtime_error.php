<?hh
/*
 * proto mixed microtime([bool get_as_float])
 * Function is implemented in ext/standard/microtime.c
*/
<<__EntryPoint>> function main(): void {
$opt_arg_0 = true;
$extra_arg = 1;

echo "\n-- Too many arguments --\n";
try { var_dump(microtime($opt_arg_0, $extra_arg)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }


echo "===DONE===\n";
}
