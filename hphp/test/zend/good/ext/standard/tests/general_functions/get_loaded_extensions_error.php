<?hh
/* Prototype  : array get_loaded_extensions  ([ bool $zend_extensions= false  ] )
 * Description:  Returns an array with the names of all modules compiled and loaded
 * Source code: Zend/zend_builtin_functions.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing get_loaded_extensions() : error conditions ***\n";

echo "\n-- Testing get_loaded_extensions() function with more than expected no. of arguments --\n";
$res = fopen(__FILE__, "r");
$extra_arg = 10;
try { var_dump( get_resource_type(true, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
