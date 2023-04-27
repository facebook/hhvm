<?hh
/* Prototype  : proto array get_declared_interfaces()
 * Description: Returns an array of all declared interfaces. 
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions: 
 */
<<__EntryPoint>> function get_declared_interfaces_error_001(): void {
echo "*** Testing get_declared_interfaces() : error conditions ***\n";

// One argument
echo "\n-- Testing get_declared_interfaces() function with one argument --\n";
$extra_arg = 10;;
try { var_dump( get_declared_interfaces($extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
