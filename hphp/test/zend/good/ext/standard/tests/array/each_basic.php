<?hh
/* Prototype  : array each(array $arr)
 * Description: Return the currently pointed key..value pair in the passed array,
 * and advance the pointer to the next element
 * Source code: Zend/zend_builtin_functions.c
 */

/*
 * Test basic functionality of each()
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing each() : basic functionality ***\n";

$arr = darray ['one' => 1, 0 => 'zero', 'two' => 'deux', 20 => 'twenty'];
echo "\n-- Passed array: --\n";
var_dump($arr);

echo "\n-- Initial position: --\n";
var_dump(each(inout $arr));

echo "\n-- End position: --\n";
end(inout $arr);
var_dump(each(inout $arr));

echo "\n-- Passed the end of array: --\n";
var_dump(each(inout $arr));

echo "Done";
}
