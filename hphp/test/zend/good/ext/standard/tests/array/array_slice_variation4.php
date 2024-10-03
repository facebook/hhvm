<?hh
/* Prototype  : array array_slice(array $input, int $offset [, int $length [, bool $preserve_keys]])
 * Description: Returns elements specified by offset and length
 * Source code: ext/standard/array.c
 */

/*
 * Pass different data types as $preserve_keys argument to array_slice() to test behaviour
 */

// get a class
class classA
{
  public function __toString() :mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_slice() : usage variations ***\n";

// Initialise function arguments not being substituted
$input_array = dict['one' => 1, 0 => 2, 99 => 3, 100 => 4];
$offset = 0;
$length = 3;


// unexpected values to be passed to $preserve_keys argument
$inputs = vec[
       // boolean data
/*12*/ true,
       false,
       TRUE,
       FALSE,
];

// loop through each element of $inputs to check the behavior of array_slice()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  try { var_dump( array_slice($input_array, $offset, $length, $input) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

echo "Done";
}
