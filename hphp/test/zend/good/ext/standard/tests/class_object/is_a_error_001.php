<?hh
/* Prototype  : proto bool is_a(object object, string class_name, bool allow_string)
 * Description: Returns true if the object is of this class or has this class as one of its parents
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */
<<__EntryPoint>> function is_a_error_001(): void {
echo "*** Testing is_a() : error conditions ***\n";

//Test is_a with one more than the expected number of arguments
echo "\n-- Testing is_a() function with more than expected no. of arguments --\n";
$object = new stdClass();
$class_name = 'string_val';
$allow_string = false;
$extra_arg = 10;

try { var_dump( is_a($object, $class_name, $allow_string, $object) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test is_a with one more than the expected number of arguments
echo "\n-- Testing is_a() function with non-boolean in last position --\n";
try { var_dump( is_a($object, $class_name, $object) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }


// Testing is_a with one less than the expected number of arguments
echo "\n-- Testing is_a() function with less than expected no. of arguments --\n";
$object = new stdClass();
try { var_dump( is_a($object) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
