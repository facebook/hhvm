<?php
/* Prototype  : string join( string $glue, array $pieces )
 * Description: Join array elements with a string
 * Source code: ext/standard/string.c
 * Alias of function: implode()
*/

/*
 * test join() by giving different pieces values 
*/

echo "*** Testing join() : usage variations ***\n";

$pieces_arrays = array (
  array(1, 2), // array with default keys and numrice values
  array(1.1, 2.2), // array with default keys & float values
  array( array(2), array(1)), // sub arrays
  array(false,true), // array with default keys and boolean values
  array(), // empty array
  array(NULL), // array with NULL
  array("a","aaaa","b","bbbb","c","ccccc"),
  
  // associative arrays
  array(1 => "one", 2 => "two", 3 => "three"),  // explicit numeric keys, string values
  array("one" => 1, "two" => 2, "three" => 3 ),  // string keys & numeric values
  array( 1 => 10, 2 => 20, 4 => 40, 3 => 30),  // explicit numeric keys and numeric values
  array( "one" => "ten", "two" => "twenty", "three" => "thirty"),  // string key/value
  array("one" => 1, 2 => "two", 4 => "four"),  //mixed
  
  // associative array, containing null/empty/boolean values as key/value
  array(NULL => "NULL", null => "null", "NULL" => NULL, "null" => null),
  array(true => "true", false => "false", "false" => false, "true" => true),
  array("" => "emptyd", '' => 'emptys', "emptyd" => "", 'emptys' => ''),
  array(1 => '', 2 => "", 3 => NULL, 4 => null, 5 => false, 6 => true),
  array('' => 1, "" => 2, NULL => 3, null => 4, false => 5, true => 6),
  
  // array with repetative keys
  array("One" => 1, "two" => 2, "One" => 10, "two" => 20, "three" => 3)
);

// a multichar glue value
$glue = "], [";

// loop through each $pieces_arrays element and call join() 
$iteration = 1;
for($index = 0; $index < count($pieces_arrays); $index ++) {
  echo "-- Iteration $iteration --\n";
  var_dump( join($glue, $pieces_arrays[$index]) );
  $iteration ++;
}
 
echo "Done\n";
?>