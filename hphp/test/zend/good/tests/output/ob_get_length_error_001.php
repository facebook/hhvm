<?hh
/* 
 * proto int ob_get_length(void)
 * Function is implemented in main/output.c
*/ 
<<__EntryPoint>> function main(): void {
$extra_arg = 1;

echo "\nToo many arguments\n";
try { var_dump(ob_get_length($extra_arg)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
