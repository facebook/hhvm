<?hh
/* Prototype  : proto array array_fill(int start_key, int num, mixed val)
 * Description: Create an array containing num elements starting with index start_key each initialized to val
 * Source code: ext/standard/array.c
 */

/*
 * testing array_fill() by passing different unexpected values for 'num' argument
 */

//define a class
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
  $val = 100;


  //array of different  values for 'num' argument
  $values = vec[
            // float values
  /* 1  */  2.5,
            -2.5,
            0.5e1,
            0.5E-1,
            .5,

            // array values
  /* 6  */  vec[],
            vec[0],
            vec[1],
            vec[1, 2],
            dict['color' => 'red', 'item' => 'pen'],

            // null values
  /* 11 */  NULL,
            null,

            // boolean values
  /* 13 */  true,
            false,
            TRUE,
            FALSE,

            // empty string
  /* 17 */  "",
            '',

            // string values
  /* 19 */  "string",
            'string',

            // objects
  /* 21 */  new test()
  ];

  // loop through each element of the array for num
  // check the working of array_fill
  echo "--- Testing array_fill() with different values for 'num' arg ---\n";
  $counter = 1;
  for ($index = 0; $index < count($values); $index++) {
    echo "-- Iteration $counter --\n";
    $num = $values[$index];

    try {
      var_dump(array_fill($start_key,$num,$val));
    } catch (Exception $e) {
      echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n";
    }

    $counter++;
  }

  echo "Done";
}
