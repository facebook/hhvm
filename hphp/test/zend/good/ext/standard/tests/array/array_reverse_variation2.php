<?hh
/* Prototype  : array array_reverse(array $array [, bool $preserve_keys])
 * Description: Return input as a new array with the order of the entries reversed
 * Source code: ext/standard/array.c
*/

/*
 * testing the functionality of array_reverse() by giving unexpected values for $preserve_keys argument
*/

//get a class
class classA
{
  public function __toString():mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_reverse() : usage variations ***\n";

// Initialise the array
$array = dict["a" => "green", 0 => "red", 1 => "blue", 2 => "red", 3 => "orange", 4 => "pink"];

//array of values to iterate over
$preserve_keys = vec[
       // boolean data
       true,
       false,
       TRUE,
       FALSE,
];

// loop through each element of the array $preserve_keys to check the behavior of array_reverse()
$iterator = 1;
foreach($preserve_keys as $preserve_key) {
  echo "-- Iteration $iterator --\n";
  try { var_dump( array_reverse($array, $preserve_key) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

echo "Done";
}
