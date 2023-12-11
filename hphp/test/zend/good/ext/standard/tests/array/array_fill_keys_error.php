<?hh
/* Prototype  : proto array array_fill_keys(array keys, mixed val)
 * Description: Create an array using the elements of the first parameter as keys each initialized to val 
 * Source code: ext/standard/array.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_fill_keys() : error conditions ***\n";

$keys = vec[1, 2];
$val = 1;
$extra_arg = 10;

echo "\n-- Testing array_fill_keys() function with more than expected no. of arguments --\n";
try { var_dump( array_fill_keys($keys, $val, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing array_fill_keys() function with less than expected no. of arguments --\n";
try { var_dump( array_fill_keys($keys) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing array_fill_keys() function with no arguments --\n";
try { var_dump( array_fill_keys() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
