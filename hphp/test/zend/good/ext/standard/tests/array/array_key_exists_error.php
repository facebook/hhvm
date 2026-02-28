<?hh
/* Prototype  : bool array_key_exists(mixed $key, array $search)
 * Description: Checks if the given key or index exists in the array 
 * Source code: ext/standard/array.c
 * Alias to functions: key_exists
 */

/*
 * Pass incorrect number of arguments to array_key_exists() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_key_exists() : error conditions ***\n";

//Test array_key_exists with one more than the expected number of arguments
echo "\n-- Testing array_key_exists() function with more than expected no. of arguments --\n";
$key = 1;
$search = vec[1, 2];
$extra_arg = 10;
try { var_dump( array_key_exists($key, $search, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing array_key_exists with one less than the expected number of arguments
echo "\n-- Testing array_key_exists() function with less than expected no. of arguments --\n";
$key = 1;
try { var_dump( array_key_exists($key) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
