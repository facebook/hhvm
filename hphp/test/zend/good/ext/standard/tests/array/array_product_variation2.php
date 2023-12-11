<?hh
/* Prototype  : mixed array_product(array input)
 * Description: Returns the product of the array entries 
 * Source code: ext/standard/array.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_product() : variations ***\n";

echo "\n-- Testing array_product() function with a keyed array array --\n";
var_dump( array_product(dict["bob" => 2, "janet" => 5]) );
echo "===DONE===\n";
}
