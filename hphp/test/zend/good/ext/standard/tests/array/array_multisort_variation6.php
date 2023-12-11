<?hh
/* Prototype  : bool array_multisort(array ar1 [, SORT_ASC|SORT_DESC [, SORT_REGULAR|SORT_NUMERIC|SORT_STRING|SORT_NATURAL|SORT_FLAG_CASE]] [, array ar2 [, SORT_ASC|SORT_DESC [, SORT_REGULAR|SORT_NUMERIC|SORT_STRING|SORT_NATURAL|SORT_FLAG_CASE]], ...])
 * Description: Sort multiple arrays at once similar to how ORDER BY clause works in SQL
 * Source code: ext/standard/array.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_multisort() : Testing  all array sort specifiers ***\n";

$ar = vec[ "2", "aa" , "1"];
$regular = SORT_REGULAR;
$string = SORT_STRING;
$numeric = SORT_NUMERIC;
$desc = SORT_DESC;

array_multisort3(inout $ar, inout $regular, inout $desc);
var_dump($ar);

array_multisort3(inout $ar, inout $string, inout $desc);
var_dump($ar);

array_multisort3(inout $ar, inout $numeric, inout $desc);
var_dump($ar);


echo "===DONE===\n";
}
