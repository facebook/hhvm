<?hh
/* Prototype  : array array_combine(array $keys, array $values)
 * Description: Creates an array by using the elements of the first parameter as keys 
 *              and the elements of the second as the corresponding values 
 * Source code: ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_combine() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing array_combine() function with Zero arguments --\n";
try { var_dump( array_combine() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test array_combine with one more than the expected number of arguments
echo "\n-- Testing array_combine() function with more than expected no. of arguments --\n";
$keys = vec[1, 2];
$values = vec[1, 2];
$extra_arg = 10;
try { var_dump( array_combine($keys,$values, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing array_combine with one less than the expected number of arguments
echo "\n-- Testing array_combine() function with less than expected no. of arguments --\n";
$keys = vec[1, 2];
try { var_dump( array_combine($keys) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
