<?hh
/* Prototype  : proto array get_object_vars(object obj)
 * Description: Returns an array of object properties
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing get_object_vars() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing get_object_vars() function with Zero arguments --\n";
try { var_dump( get_object_vars() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test get_object_vars with one more than the expected number of arguments
echo "\n-- Testing get_object_vars() function with more than expected no. of arguments --\n";
$obj = new stdClass();
$extra_arg = 10;
try { var_dump( get_object_vars($obj, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
