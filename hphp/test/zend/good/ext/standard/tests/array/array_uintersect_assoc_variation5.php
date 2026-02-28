<?hh
/* Prototype  : array array_uintersect_assoc(array arr1, array arr2 [, array ...], callback data_compare_func)
 * Description: Returns the entries of arr1 that have values which are present in all the other arguments. Data is compared by using an user-supplied callback.
 * Source code: ext/standard/array.c
 * Alias to functions:
 */
function incorrect_return_value ($val1, $val2) :mixed{
  return vec[1];
}
function too_many_parameters ($val1, $val2, $val3) :mixed{
  return 1;
}
function too_few_parameters ($val1) :mixed{
  return 1;
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_uintersect_assoc() : usage variation - differing comparison functions***\n";

$arr1 = vec[1];
$arr2 = vec[1,2];

echo "\n-- comparison function with an incorrect return value --\n";
var_dump(array_uintersect_assoc($arr1, $arr2, incorrect_return_value<>));

echo "\n-- comparison function taking too many parameters --\n";
try { var_dump(array_uintersect_assoc($arr1, $arr2, too_many_parameters<>)); } catch (Exception $e) { var_dump($e->getMessage()); }

echo "\n-- comparison function taking too few parameters --\n";
var_dump(array_uintersect_assoc($arr1, $arr2, too_few_parameters<>));

echo "===DONE===\n";
}
