<?hh
/* Prototype  : array array_udiff_assoc(array arr1, array arr2 [, array ...], callback key_comp_func)
 * Description: Returns the entries of arr1 that have values which are not present in any of the others arguments but do additional checks whether the keys are equal. Keys are compared by user supplied function.
 * Source code: ext/standard/array.c
 * Alias to functions:
 */

function incorrect_return_value ($val1, $val2) {
  return varray[1];
}
function too_many_parameters ($val1, $val2, $val3) {
  return 1;
}
function too_few_parameters ($val1) {
  return 1;
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_udiff_assoc() : usage variation - differing comparison functions***\n";

$arr1 = varray[1];
$arr2 = varray[1,2];

echo "\n-- comparison function with an incorrect return value --\n";
var_dump(array_udiff_assoc($arr1, $arr2, fun('incorrect_return_value')));

echo "\n-- comparison function taking too many parameters --\n";
try { var_dump(array_udiff_assoc($arr1, $arr2, fun('too_many_parameters'))); } catch (Exception $e) { var_dump($e->getMessage()); }

echo "\n-- comparison function taking too few parameters --\n";
var_dump(array_udiff_assoc($arr1, $arr2, fun('too_few_parameters')));


echo "===DONE===\n";
}
