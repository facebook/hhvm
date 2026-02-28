<?hh
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays
 * Source code: ext/standard/array.c
 */

/*
 * Test array_map() by passing non-permmited built-in functions and language constructs i.e.
 *   echo(), array(), eval(), exit(), isset(), list(), print()
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_map() : non-permmited built-in functions ***\n";

// array to be passed as arguments
$arr1 = vec[1, 2];

// built-in functions & language constructs
$callback_names = vec[
/*1*/  'echo',
       'array',
/*3*/  'eval',
       'exit',
       'isset',
       'list',
/*7*/  'print'
];
for($count = 0; $count < count($callback_names); $count++)
{
  echo "-- Iteration ".($count + 1)." --\n";
  var_dump( array_map($callback_names[$count], $arr1) );
}

echo "Done";
}
