<?hh
/* Prototype  : proto array get_declared_classes()
 * Description: Returns an array of all declared classes.
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */
<<__EntryPoint>> function get_declared_classes_error_001(): void {
echo "*** Testing get_declared_classes() : error conditions ***\n";

// One argument
echo "\n-- Testing get_declared_classes() function with one argument --\n";
$extra_arg = 10;;
try { var_dump( get_declared_classes($extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
