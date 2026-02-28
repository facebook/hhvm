<?hh
/* Prototype  : int array_push(&array $stack, mixed $var [, mixed $...])
 * Description: Pushes elements onto the end of the array
 * Source code: ext/standard/array.c
 */

/*
 * Test array_push when passed:
 * 1. an array as $var arg
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_push() : usage variations ***\n";

echo "\n-- Pass array as \$var argument --\n";
$three = 3;
$array = vec[1, 2, 3];
$sub_array = vec['one', 'two'];
var_dump(array_push(inout $array, $sub_array));
var_dump($array);

echo "Done";
}
