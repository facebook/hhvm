<?hh
/* Prototype  : array array_combine(array $keys, array $values)
 * Description: Creates an array by using the elements of the first parameter as keys 
 *              and the elements of the second as the corresponding values 
 * Source code: ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_combine() : error conditions specific to array_combine() ***\n";

// Testing array_combine by passing empty arrays to $keys and $values arguments
echo "\n-- Testing array_combine() function with empty arrays --\n";
var_dump( array_combine(varray[], varray[]) );

// Testing array_combine by passing empty array to $keys
echo "\n-- Testing array_combine() function with empty array for \$keys argument --\n";
var_dump( array_combine(varray[], varray[1, 2]) );

// Testing array_combine by passing empty array to $values
echo "\n-- Testing array_combine() function with empty array for \$values argument --\n";
var_dump( array_combine(varray[1, 2], varray[]) );

// Testing array_combine with arrays having unequal number of elements
echo "\n-- Testing array_combine() function by passing array with unequal number of elements --\n";
var_dump( array_combine(varray[1, 2], varray[1, 2, 3]) );

echo "Done";
}
