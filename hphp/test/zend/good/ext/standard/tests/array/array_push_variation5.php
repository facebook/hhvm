<?hh
/* Prototype  : int array_push(&array $stack, mixed $var [, mixed $...])
 * Description: Pushes elements onto the end of the array
 * Source code: ext/standard/array.c
 */

/*
 * Check the position of the internal array pointer after calling array_push()
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_push() : usage variations ***\n";

$stack = dict['one' => 'un', 'two' => 'deux'];
$var0 = 'zero';

echo "\n-- Call array_push() --\n";
$result = array_push(inout $stack, $var0);
var_dump($result);
echo "Done";
}
