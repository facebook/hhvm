<?hh
/* Prototype  : mixed array_rand(array input [, int num_req])
 * Description: Return key/keys for random entry/entries in the array 
 * Source code: ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_rand() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing array_rand() function with Zero arguments --\n";
try { var_dump( array_rand() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test array_rand with one more than the expected number of arguments
echo "\n-- Testing array_rand() function with more than expected no. of arguments --\n";
$input = vec[1, 2];
$num_req = 10;
$extra_arg = 10;
try { var_dump( array_rand($input,$num_req, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
