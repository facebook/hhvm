<?hh
/* Prototype  : string join( string $glue, array $pieces )
 * Description: Join array elements with a string
 * Source code: ext/standard/string.c
 * Alias of function: implode()
*/

/*
 * test join() by passing different unexpected value for pieces argument
*/

// define a class
class test {
  public $t = 10;
  public $p = 10;
  function __toString() :mixed{
    return "testObject";
  }
}

<<__EntryPoint>>
function main(): void {
  echo "*** Testing join() : usage variations ***\n";
  // initialize all required variables
  $glue = '::';


  // get a resouce variable
  $fp = fopen(__FILE__, "r");

  // array with different values
  $values = vec[
    // integer values
    0,
    1,
    12345,
    -2345,

    // float values
    10.5,
    -10.5,
    10.5e10,
    10.6E-10,
    .5,

    // boolean values
    true,
    false,
    TRUE,
    FALSE,

    // string values
    "string",
    'string',

    // objects
    new test(),

    // empty string
    "",
    '',

    // null vlaues
    NULL,
    null,

    // resource variable
    $fp,
  ];

  // loop through each element of the array and check the working of join()
  // when $pieces argument is supplied with different values
  echo "\n--- Testing join() by supplying different values for 'pieces' argument ---\n";
  $counter = 1;
  for ($index = 0; $index < count($values); $index++) {
    echo "-- Iteration $counter --\n";
    $pieces = $values[$index];

    var_dump(join($glue, $pieces));

    $counter++;
  }

  // close the resources used
  fclose($fp);

  echo "Done\n";
}
