<?hh
/* Prototype  : mixed array_product(array input)
 * Description: Returns the product of the array entries 
 * Source code: ext/standard/array.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_product() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing array_product() function with Zero arguments --\n";
try { var_dump( array_product() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test array_product with one more than the expected number of arguments
echo "\n-- Testing array_product() function with more than expected no. of arguments --\n";
$input = vec[1, 2];
$extra_arg = 10;
try { var_dump( array_product($input, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing array_product() function incorrect argument type --\n";
var_dump( array_product("bob") );

echo "===DONE===\n";
}
