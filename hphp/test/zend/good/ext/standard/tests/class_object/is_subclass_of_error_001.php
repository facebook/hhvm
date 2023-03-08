<?hh
/* Prototype  : proto bool is_subclass_of(object object, string class_name)
 * Description: Returns true if the object has this class as one of its parents
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */
<<__EntryPoint>> function is_subclass_of_error_001(): void {
echo "*** Testing is_subclass_of() : error conditions ***\n";


//Test is_subclass_of with one more than the expected number of arguments
echo "\n-- Testing is_subclass_of() function with more than expected no. of arguments --\n";
$object = new stdClass();
$class_name = 'string_val';
$allow_string = false;
$extra_arg = 10;
try { var_dump( is_subclass_of($object, $class_name, $allow_string, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test is_subclass_of with invalid last argument
echo "\n-- Testing is_subclass_of() function with more than typo style invalid 3rd argument --\n";
try { var_dump( is_subclass_of($object, $class_name, $class_name) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }


//Test is_subclass_of with invalid last argument
echo "\n-- Testing is_subclass_of() function with more than invalid 3rd argument --\n";
try { var_dump( is_subclass_of($object, $class_name, $object) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing is_subclass_of with one less than the expected number of arguments
echo "\n-- Testing is_subclass_of() function with less than expected no. of arguments --\n";
$object = new stdClass();
try { var_dump( is_subclass_of($object) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
