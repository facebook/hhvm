<?hh
/* Prototype  : bool array_multisort(array ar1 [, SORT_ASC|SORT_DESC [, SORT_REGULAR|SORT_NUMERIC|SORT_STRING|SORT_NATURAL|SORT_FLAG_CASE]] [, array ar2 [, SORT_ASC|SORT_DESC [, SORT_REGULAR|SORT_NUMERIC|SORT_STRING|SORT_NATURAL|SORT_FLAG_CASE]], ...])
 * Description: Sort multiple arrays at once similar to how ORDER BY clause works in SQL
 * Source code: ext/standard/array.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_multisort() : basic functionality - renumbering of numeric keys ***\n";

// Initialise all required variables
$ar1 = array( "strkey" => 2,  1,  9 => 1);
$ar2 = array( 2, "aa" , "1");

echo "\n-- Testing array_multisort() function with all normal arguments --\n";
$asc = SORT_ASC;
$regular = SORT_REGULAR;
$numeric = SORT_NUMERIC;
var_dump(array_multisort6(&$ar1, &$asc, &$regular, &$ar2, &$asc, &$numeric));
var_dump($ar1, $ar2);

echo "===DONE===\n";
}
