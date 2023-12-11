<?hh
/* Prototype  : proto array array_chunk(array $array, int $size [, bool $preserve_keys])
 * Description: Split array into chunks
 *              Chunks an array into size  large chunks.
 * Source code: ext/standard/array.c
*/

/*
* Testing array_chunk() function with unexpected values for 'array' argument
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_chunk() : usage variations ***\n";

// Initialise function arguments
$size = 10;


//array of values to iterate over
$values = vec[

        // int data
/*1*/   0,
        1,
        12345,
        -2345,

        // float data
/*5*/   10.5,
        -10.5,
        10.5e10,
        10.6E-10,
        .5,

        // null data
/*10*/  NULL,
        null,

        // boolean data
/*12*/  true,
        false,
        TRUE,
        FALSE,

        // empty data
/*16*/  "",
        '',

        // string data
/*18*/  "string",
        'string',

        // object data
/*20*/  new stdClass(),

];

$count = 1;
// loop through each element of the array for input
foreach($values as $value){
  echo "\n-- Iteration $count --\n";
  var_dump( array_chunk($value, $size) );
  var_dump( array_chunk($value, $size, true) );
  var_dump( array_chunk($value, $size, false) );
  $count++;
}

echo "Done";
}
