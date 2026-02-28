<?hh
/* Prototype  : int array_push(&array $stack, mixed $var [, mixed $...])
 * Description: Pushes elements onto the end of the array
 * Source code: ext/standard/array.c
 */

/*
 * Test basic functionality of array_push with indexed and associative arrays
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_push() : basic functionality ***\n";

$array = vec['zero', 'one', 'two'];
$var1 = 'three';
$var2 = 'four';

echo "\n-- Push values onto an indexed array --\n";
var_dump(array_push(inout $array, $var1, $var2));
var_dump($array);

$array_assoc = dict['one' => 'un', 'two' => 'deux'];

echo "\n-- Push values onto an associative array --\n";
var_dump(array_push(inout $array_assoc, $var1, $var2));
var_dump($array_assoc);

echo "Done";
}
