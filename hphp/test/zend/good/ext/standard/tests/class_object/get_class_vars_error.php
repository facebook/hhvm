<?hh
/* Prototype  : array get_class_vars(string class_name)
 * Description: Returns an array of default properties of the class.
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing get_class_vars() : error conditions ***\n";


//Test get_class_vars with one more than the expected number of arguments
echo "\n-- Testing get_class_vars() function with more than expected no. of arguments --\n";
$obj = new stdClass();
$extra_arg = 10;
try { var_dump(get_class_vars($obj,$extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing get_class_vars with one less than the expected number of arguments
echo "\n-- Testing get_class_vars() function with less than expected no. of arguments --\n";
try { var_dump(get_class_vars()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
