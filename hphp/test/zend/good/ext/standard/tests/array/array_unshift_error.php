<?hh
/* Prototype  : int array_unshift(array $array, mixed $var [, mixed ...])
 * Description: Pushes elements onto the beginning of the array
 * Source code: ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_unshift() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing array_unshift() function with Zero arguments --\n";
try { var_dump( array_unshift() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing array_unshift with one less than the expected number of arguments
echo "\n-- Testing array_unshift() function with less than expected no. of arguments --\n";
$array = vec[1, 2];
try { var_dump( array_unshift(inout $array, ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "Done";
}
