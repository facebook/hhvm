<?hh
/* Prototype  : array get_extension_funcs  ( string $module_name  )
 * Description: Returns an array with the names of the functions of a module.
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing get_extension_funcs() : error conditions ***\n";

echo "\n-- Too few arguments --\n";
try { var_dump(get_extension_funcs()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

$extra_arg = 1;
echo "\n-- Too many arguments --\n";
try { var_dump(get_extension_funcs("standard", $extra_arg)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Invalid extension name --\n";
var_dump(get_extension_funcs("foo"));

echo "===DONE===\n";
}
