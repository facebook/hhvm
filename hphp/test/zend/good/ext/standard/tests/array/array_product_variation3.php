<?hh
/* Prototype  : mixed array_product(array input)
 * Description: Returns the product of the array entries 
 * Source code: ext/standard/array.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_product() : variations - negative numbers***\n";

echo "\n-- Testing array_product() function with one negative number --\n";
var_dump( array_product(vec[-2]) );

echo "\n-- Testing array_product() function with two negative numbers --\n";
var_dump( array_product(vec[-2, -3]) );

echo "\n-- Testing array_product() function with three negative numbers --\n";
var_dump( array_product(vec[-2, -3, -4]) );

echo "\n-- Testing array_product() function with negative floats --\n";
var_dump( array_product(vec[-1.5]));

echo "\n-- Testing array_product() function with negative floats --\n";
var_dump( array_product(vec[-99999999.9, 99999999.1]));


echo "===DONE===\n";
}
