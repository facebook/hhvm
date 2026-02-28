<?hh
/* Prototype:
 *  array array_column(array $input, mixed $column_key[, mixed $index_key]);
 * Description:
 *  Returns an array containing all the values from
 *  the specified "column" in a two-dimensional array.
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_column() : error conditions ***\n";

echo "\n-- Testing array_column() function with Zero arguments --\n";
try { var_dump(array_column()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing array_column() function with One argument --\n";
try { var_dump(array_column(vec[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing array_column() function with string as first parameter --\n";
var_dump(array_column('foo', 0));

echo "\n-- Testing array_column() function with int as first parameter --\n";
var_dump(array_column(1, 'foo'));

echo "\n-- Testing array_column() column key parameter should be a string or an integer (testing bool) --\n";
var_dump(array_column(vec[], true));

echo "\n-- Testing array_column() column key parameter should be a string or integer (testing array) --\n";
var_dump(array_column(vec[], vec[]));

echo "\n-- Testing array_column() index key parameter should be a string or an integer (testing bool) --\n";
var_dump(array_column(vec[], 'foo', true));

echo "\n-- Testing array_column() index key parameter should be a string or integer (testing array) --\n";
var_dump(array_column(vec[], 'foo', vec[]));

echo "Done\n";
}
