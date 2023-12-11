<?hh
/* Prototype  : array array_filter(array $input [, callback $callback])
 * Description: Filters elements from the array via the callback.
 * Source code: ext/standard/array.c
*/

/*
* Passing built-in functions and different language constructs as 'callback' argument
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_filter() : usage variations - built-in functions as 'callback' argument ***\n";

$input = vec[0, 1, -1, 10, 100, 1000, 'Hello', null];

// using built-in function 'is_int' as 'callback'
var_dump( array_filter($input, 'is_int') );

// using built-in function 'chr' as 'callback'
var_dump( array_filter($input, 'chr') );

// using language construct 'echo' as 'callback'
var_dump( array_filter($input, 'echo') );

// using language construct 'exit' as 'callback'
var_dump( array_filter($input, 'exit') );

echo "Done";
}
