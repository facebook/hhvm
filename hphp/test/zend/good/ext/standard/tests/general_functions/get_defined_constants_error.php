<?hh
/* Prototype  : array get_defined_constants  ([ bool $categorize  ] )
 * Description:  Returns an associative array with the names of all the constants and their values
 * Source code: Zend/zend_builtin_functions.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing get_defined_constants() : error conditions ***\n";

echo "\n-- Testing get_defined_constants() function with more than expected no. of arguments --\n";
$extra_arg = 10;
try { var_dump( get_defined_constants(true, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
