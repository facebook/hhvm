<?hh
/* Prototype  : array array_chunk(array input, int size [, bool preserve_keys])
 * Description: Split array into chunks 
 * Source code: ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_chunk() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing array_chunk() function with zero arguments --\n";
try { var_dump( array_chunk() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing array_chunk() function with more than expected no. of arguments --\n";
$input = vec[1, 2];
$size = 10;
$preserve_keys = true;
$extra_arg = 10;
try { var_dump( array_chunk($input,$size,$preserve_keys, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing array_chunk() function with less than expected no. of arguments --\n";
$input = vec[1, 2];
try { var_dump( array_chunk($input) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
