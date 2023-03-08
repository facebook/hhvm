<?hh
/* Prototype  : proto string get_parent_class([mixed object])
 * Description: Retrieves the parent class name for object or class or current scope. 
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions: 
 */
<<__EntryPoint>> function get_parent_class_error_001(): void {
echo "*** Testing get_parent_class() : error conditions ***\n";


//Test get_parent_class with one more than the expected number of arguments
echo "\n-- Testing get_parent_class() function with more than expected no. of arguments --\n";
$object = 1;
$extra_arg = 10;
try { var_dump( get_parent_class($object, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
