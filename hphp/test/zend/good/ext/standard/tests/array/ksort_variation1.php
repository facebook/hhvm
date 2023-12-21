<?hh
/* Prototype  : bool ksort(&array &array [, int sort_flags])
 * Description: Sort an array by key, maintaining key to data correlation
 * Source code: ext/standard/array.c
*/

/*
 * testing ksort() by providing different unexpected values for array argument with following flag values:
 *  1. flag value as defualt
 *  2. SORT_REGULAR - compare items normally
 *  3. SORT_NUMERIC - compare items numerically
 *  4. SORT_STRING - compare items as strings
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing ksort() : usage variations ***\n";


// resource variable
$fp = fopen(__FILE__, "r");

$unexpected_values = vec[

        // int data
/*1*/   0,
        1,
        12345,
        -2345,

        // float data
/*5*/   10.5,
        -10.5,
        10.5e3,
        10.6E-2,
        0.5,

        // null data
/*10*/  NULL,
        null,

        // boolean data
/*11*/  true,
        false,
        TRUE,
        FALSE,

        // empty data
/*15*/  "",
        '',

        // string data
/*17*/  "string",
        'string',

        // object data
/*19*/  new stdClass(),

        // resource variable
/*20*/  $fp

];

// loop though each element of the array and check the working of ksort()
// when $array argument is supplied with different values from $unexpected_values
echo "\n-- Testing ksort() by supplying different unexpected values for 'array' argument --\n";
echo "\n-- Flag values are defualt, SORT_REGULAR, SORT_NUMERIC, SORT_STRING --\n";

$counter = 1;
for($index = 0; $index < count($unexpected_values); $index ++) {
  echo "-- Iteration $counter --\n";
  $value = $unexpected_values [$index];
  var_dump( ksort(inout $value) ); // expecting : bool(false)
  var_dump( ksort(inout $value, SORT_REGULAR) ); // expecting : bool(false)
  var_dump( ksort(inout $value, SORT_NUMERIC) ); // expecting : bool(false)
  var_dump( ksort(inout $value, SORT_STRING) ); // expecting : bool(false)
  $counter++;
}

echo "Done";
}
