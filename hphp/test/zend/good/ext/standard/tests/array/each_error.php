<?hh
/* Prototype  : array each(&array $arr)
 * Description: Return the currently pointed key..value pair in the passed array,
 * and advance the pointer to the next element
 * Source code: Zend/zend_builtin_functions.c
 */

/*
 * Pass an incorrect number of arguments to each() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing each() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing each() function with Zero arguments --\n";
try { var_dump( each() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test each with one more than the expected number of arguments
echo "\n-- Testing each() function with more than expected no. of arguments --\n";
$arr = varray[1, 2];
$extra_arg = 10;
try { var_dump( each(inout $arr, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
