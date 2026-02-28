<?hh
/* Prototype  : array array_fill(int $start_key, int $num, mixed $val)
 * Description: Create an array containing num elements starting with index start_key each initialized to val
 * Source code: ext/standard/array.c
 */

/*
 * testing array_fill() by passing different unexpected values for 'val' argument
 */

// define a class
class test {
  public $t = 10;
  function __toString() :mixed{
    return "testObject";
  }
}

<<__EntryPoint>>
function main(): void {
  echo "*** Testing array_fill() : usage variations ***\n";

  // Initialise function arguments not being substituted
  $start_key = 0;
  $num = 2;


  //array of different values for 'val' argument
  $values = vec[
            // empty string
  /* 1  */  "",
            '',
            // objects
  /* 3  */  new test(),
  ];

  // loop through each element of the array for 'val' argument
  // check the working of array_fill()
  echo "--- Testing array_fill() with different values for 'val' argument ---\n";
  $counter = 1;
  for($index = 0; $index < count($values); $index ++) {
    echo "-- Iteration $counter --\n";
    $val = $values[$index];

    var_dump( array_fill($start_key , $num , $val) );

    $counter++;
  }

  echo"Done";
}
