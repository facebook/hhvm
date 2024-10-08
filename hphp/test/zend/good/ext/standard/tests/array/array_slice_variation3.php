<?hh
/* Prototype  : array array_slice(array $input, int $offset [, int $length [, bool $preserve_keys]])
 * Description: Returns elements specified by offset and length
 * Source code: ext/standard/array.c
 */

/*
 * Pass different data types as $length argument to array_slice to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_slice() : usage variations ***\n";

// Initialise function arguments not being substituted
$input_array = dict['one' => 1, 0 => 2, 'three' => 3, 1 => 4];
$offset = 2;


// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// unexpected values to be passed to $length argument
$inputs = vec[

       // int data
/*1*/  0,
       1,
       12345,
       -2345,

       // float data
/*5*/  10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
       .5,

       // null data
/*10*/ NULL,
       null,

       // boolean data
/*12*/ true,
       false,
       TRUE,
       FALSE,

       // empty data
/*16*/ "",
       '',
       vec[],

       // string data
/*19*/ "string",
       'string',
       $heredoc,


];

// loop through each element of $inputs to check the behavior of array_slice
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  var_dump( array_slice($input_array, $offset, $input) );
  $iterator++;
};

echo "Done";
}
