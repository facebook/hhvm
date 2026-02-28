<?hh
/* Prototype  : int count(mixed $var [, int $mode])
 * Description: Count the number of elements in a variable (usually an array)
 * Source code: ext/standard/array.c
 */

/*
 * Pass different data types as $mode argument to count() to test behaviour
 */

// get a class
class classA
{
  public function __toString() :mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing count() : usage variations ***\n";

// Initialise function arguments not being substituted
$var = vec[1, 2, vec['one', 'two']];


// unexpected values to be passed to $mode argument
$inputs = vec[
       // int data
/*1*/  0,
       1,
       12345,
       -2345,
];

// loop through each element of $inputs to check the behavior of count()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  try { var_dump( count($var, $input) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

echo "Done";
}
