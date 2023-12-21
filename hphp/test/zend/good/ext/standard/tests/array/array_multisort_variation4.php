<?hh
/* Prototype  : bool array_multisort(array ar1 [, SORT_ASC|SORT_DESC [, SORT_REGULAR|SORT_NUMERIC|SORT_STRING|SORT_NATURAL|SORT_FLAG_CASE]] [, array ar2 [, SORT_ASC|SORT_DESC [, SORT_REGULAR|SORT_NUMERIC|SORT_STRING|SORT_NATURAL|SORT_FLAG_CASE]], ...])
 * Description: Sort multiple arrays at once similar to how ORDER BY clause works in SQL
 * Source code: ext/standard/array.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_multisort() : Testing  with multiple array arguments ***\n";

$arr1 = vec[4,3,3,3];
$arr2 = vec[9,3,2,2];
$arr3 = vec[9,9,2,1];

var_dump(array_multisort3(inout $arr1, inout $arr2, inout $arr3));

var_dump($arr1);
var_dump($arr2);
var_dump($arr3);

echo "===DONE===\n";
}
