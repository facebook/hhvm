<?hh
/* Prototype  : array array_reverse(array $array [, bool $preserve_keys])
 * Description: Return input as a new array with the order of the entries reversed 
 * Source code: ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_reverse() : error conditions ***\n";

// zero arguments
echo "\n-- Testing array_reverse() function with Zero arguments --\n";
try { var_dump( array_reverse() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// more than the expected number of arguments
echo "\n-- Testing array_diff() function with more than expected no. of arguments --\n";
$array = vec[1, 2, 3, 4, 5, 6];
$extra_arg = 10;
try { var_dump( array_reverse($array, true, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( array_reverse($array, false, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
