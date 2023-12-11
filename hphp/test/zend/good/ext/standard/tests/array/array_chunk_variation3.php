<?hh
/* Prototype  : array array_chunk(array $array, int $size [, bool $preserve_keys])
 * Description: Split array into chunks
 * Source code: ext/standard/array.c
*/

/*
* Testing array_chunk() function with unexpected values for 'preserve_keys'
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_chunk() : usage variations ***\n";

// input array
$input = vec[1, 2];
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

        // empty data
/*12*/  "",
        '',

        // string data
/*14*/  "string",
        'string',

        // object data
/*16*/  new stdClass(),

];

$count = 1;

// loop through each element of the array for preserve_keys
foreach($values as $value) {
  echo "\n-- Iteration $count --\n";
  try { var_dump( array_chunk($input, $size, $value) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $count++;
}

echo "Done";
}
