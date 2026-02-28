<?hh
/* Prototype  : array array_pad(array $input, int $pad_size, mixed $pad_value)
 * Description: Returns a copy of input array padded with pad_value to size pad_size 
 * Source code: ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_pad() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing array_pad() function with Zero arguments --\n";
try { var_dump( array_pad() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test array_pad with one more than the expected number of arguments
echo "\n-- Testing array_pad() function with more than expected no. of arguments --\n";
$input = vec[1, 2];
$pad_size = 10;
$pad_value = 1;
$extra_arg = 10;
try { var_dump( array_pad($input, $pad_size, $pad_value, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing array_pad with less than the expected number of arguments
echo "\n-- Testing array_pad() function with less than expected no. of arguments --\n";
$input = vec[1, 2];
$pad_size = 10;
try { var_dump( array_pad($input, $pad_size) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( array_pad($input) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
