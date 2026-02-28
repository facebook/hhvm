<?hh
/* Prototype  : bool array_multisort(array ar1 [, SORT_ASC|SORT_DESC [, SORT_REGULAR|SORT_NUMERIC|SORT_STRING|SORT_NATURAL|SORT_FLAG_CASE]] [, array ar2 [, SORT_ASC|SORT_DESC [, SORT_REGULAR|SORT_NUMERIC|SORT_STRING|SORT_NATURAL|SORT_FLAG_CASE]], ...])
 * Description: Sort multiple arrays at once similar to how ORDER BY clause works in SQL
 * Source code: ext/standard/array.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_multisort() : basic functionality ***\n";

// Initialise all required variables
$ar1 = dict["row1" => 2, "row2" => 1, "row3" => 1];
$ar2 = dict["row1" => "2", "row2" => "aa", "row3" => "1"];

echo "\n-- Testing array_multisort() function with all normal arguments --\n";
$asc = SORT_ASC;
$regular = SORT_REGULAR;
$desc = SORT_DESC;
$string = SORT_STRING;
var_dump(array_multisort6(inout $ar1, inout $asc, inout $regular, inout $ar2, inout $desc, inout $string));
var_dump($ar1, $ar2);

echo "\n-- Testing array_multisort() function with one argument --\n";
var_dump( array_multisort1(inout $ar2) );
var_dump($ar2);


echo "===DONE===\n";
}
