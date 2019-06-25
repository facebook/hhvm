<?hh

/* Prototype  : array get_defined_functions  ( void  )
 * Description: Gets an array of all defined functions.
 * Source code: Zend/zend_builtin_functions.c
*/

<<__EntryPoint>> function main(): void {
echo "*** Testing get_defined_functions() : error conditions ***\n";


echo "\n-- Testing get_defined_functions() function with more than expected no. of arguments --\n";
$extra_arg = 10;
try { var_dump( get_defined_functions($extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n===Done===\n";
}
