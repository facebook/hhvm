<?hh
/* Prototype  : array array_flip(array $input)
 * Description: Return array with key <-> value flipped 
 * Source code: ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_flip() : error conditions ***\n";

// Zero arguments
echo "-- Testing array_flip() function with Zero arguments --\n";
try { var_dump( array_flip() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//one more than the expected number of arguments
echo "-- Testing array_flip() function with more than expected no. of arguments --\n";
$input = dict[1 => 'one', 2 => 'two'];
$extra_arg = 10;
try { var_dump( array_flip($input, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
