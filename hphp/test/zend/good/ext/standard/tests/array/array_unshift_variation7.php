<?hh
/* Prototype  : int array_unshift(array $array, mixed $var [, mixed ...])
 * Description: Pushes elements onto the beginning of the array
 * Source code: ext/standard/array.c
*/

/*
 * Testing the functionality of array_unshift() by passing different
 * double quoted strings for $var argument that is prepended to the array
 * passed through $array argument
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_unshift() : double quoted strings for \$var argument ***\n";

// array to be passed to $array argument
$array = dict['f' => "first", "s" => 'second', 0 => 1, 1 => 2.222];

// different variations of double quoted strings to be passed to $var argument
$vars = vec[
  "\$ -> This represents the dollar sign. hello dollar!!!",
  "\t\r\v The quick brown fo\fx jumped over the lazy dog",
  "This is a text with special chars: \!\@\#\$\%\^\&\*\(\)\\",
  "hello world\\t",
  "This is \ta text in bold letters\r\s\malong with slashes\n : HELLO WORLD\t"
];

// loop through the various elements of $arrays to test array_unshift()
$iterator = 1;
foreach($vars as $var) {
  echo "-- Iteration $iterator --\n";
  $temp_array = $array;  // assign $array to another temporary $temp_array

  /* with default argument */
  // returns element count in the resulting array after arguments are pushed to
  // beginning of the given array
  var_dump( array_unshift(inout $temp_array, $var) );

// dump the resulting array
  var_dump($temp_array);

  /* with optional arguments */
  // returns element count in the resulting array after arguments are pushed to
  // beginning of the given array
  $temp_array = $array;
  var_dump( array_unshift(inout $temp_array, $var, "hello", 'world') );

  // dump the resulting array
  var_dump($temp_array);
  $iterator++;
}

echo "Done";
}
