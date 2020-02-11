<?hh
/* Prototype  : mixed key(array $array_arg)
 * Description: Return the key of the element currently pointed to by the internal array pointer
 * Source code: ext/standard/array.c
 */

/*
 * Test how key() behaves with muti-dimensional and recursive arrays
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing key() : usage variations ***\n";

echo "\n-- Two Dimensional Array --\n";
$multi_array = varray ['zero', varray [1, 2, 3], 'two'];
echo "Initial Position: ";
var_dump(key($multi_array));

echo "Next Position:    ";
next(inout $multi_array);
var_dump(key($multi_array));

echo "End Position:     ";
end(inout $multi_array);
var_dump(key($multi_array));

echo "===DONE===\n";
}
