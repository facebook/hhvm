<?hh
/* Prototype  : mixed current(array $array_arg)
 * Description: Return the element currently pointed to by the internal array pointer
 * Source code: ext/standard/array.c
 * Alias to functions: pos
 */

/*
 * Test how current() behaves with muti-dimensional and recursive arrays
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing current() : usage variations ***\n";

echo "\n-- Two Dimensional Array --\n";
$multi_array = varray ['zero', varray [1, 2, 3], 'two'];
echo "Initial Position: ";
var_dump(current($multi_array));

echo "Next Position:    ";
next(inout $multi_array);
var_dump(current($multi_array));

echo "End Position:     ";
end(inout $multi_array);
var_dump(current($multi_array));

echo "===DONE===\n";
}
