<?hh
/* Prototype  : proto bool method_exists(object object, string method)
 * Description: Checks if the class method exists
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing method_exists() : error conditions ***\n";


//Test method_exists with one more than the expected number of arguments
echo "\n-- Testing method_exists() function with more than expected no. of arguments --\n";
$object = new stdClass();
$method = 'string_val';
$extra_arg = 10;
try { var_dump( method_exists($object, $method, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing method_exists with one less than the expected number of arguments
echo "\n-- Testing method_exists() function with less than expected no. of arguments --\n";
$object = new stdClass();
try { var_dump( method_exists($object) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
