<?hh
/* Prototype  : int array_push(&array $stack, mixed $var [, mixed $...])
 * Description: Pushes elements onto the end of the array
 * Source code: ext/standard/array.c
 */

/*
 * Pass incorrect number of arguments to array_push() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_push() : error conditions ***\n";

// Testing array_push with one less than the expected number of arguments
echo "\n-- Testing array_push() function with less than expected no. of arguments --\n";
$stack = vec[1, 2];
try { var_dump( array_push(inout $stack, ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
