<?hh
/* Prototype  : mixed array_rand(array input [, int num_req])
 * Description: Return key/keys for random entry/entries in the array
 * Source code: ext/standard/array.c
*/

/*
* Test array_rand() with different types of values other than int passed to 'num_req' argument
* to see that function works with unexpeced data and generates warning message as required.
*/

//define a class
class test {
  public $t = 10;
  function __toString() :mixed{
    return "3object";
  }
}

<<__EntryPoint>>
function main(): void {
  echo "*** Testing array_rand() : unexpected values for 'num_req' parameter ***\n";

  // Initialise function arguments
  $input = vec[1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13];


  //array of values to iterate over
  $values = vec[
          // int data
  /*1*/   0,
          1,
          12345,
          -2345,
  ];

  // loop through each element of the array for different values for 'num_req' argument
  $count = 1;
  foreach ($values as $value) {
    echo "\n-- Iteration $count --\n";
    try {
      var_dump(array_rand($input, $value));
    } catch (Exception $e) {
      echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n";
    }
    $count++;
  }

  echo "Done";
}
