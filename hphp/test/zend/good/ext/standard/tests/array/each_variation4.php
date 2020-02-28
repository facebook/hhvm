<?hh
/* Prototype  : array each(&array $arr)
 * Description: Return the currently pointed key..value pair in the passed array,
 * and advance the pointer to the next element
 * Source code: Zend/zend_builtin_functions.c
 */

/*
 * Test behaviour of each() when:
 * 1. Passed an array
 * 2. Passed an array as $arr argument by reference
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing each() : usage variations ***\n";

$val1 = 'foo';
$val2 = 'bar';

$arr1 = darray['one' => $val1, 0 => $val2];

echo "-- Call each until at the end of the array: --\n";
var_dump( each(inout $arr1) );
var_dump( each(inout $arr1) );
var_dump( each(inout $arr1) );

echo "Done";
}
