<?hh
/* Prototype  : mixed array_rand(array input [, int num_req])
 * Description: Return key/keys for random entry/entries in the array
 * Source code: ext/standard/array.c
*/

/*
* Test array_rand() with different types of values other than arrays passed to the 'input' parameter
* to see that function works with unexpeced data and generates warning message as required.
*/

//define a class
class test {
  public $t = 10;
  function __toString() :mixed{
    return "object";
  }
}

<<__EntryPoint>>
function main(): void {
  echo "*** Testing array_rand() : unexpected values for 'input' parameter ***\n";

  // Initialise function arguments
  $num_req = 10;


  //get a resource variable
  $fp = fopen(__FILE__, "r");

  //array of different values for 'input' parameter
  $values = vec[
          // int data
  /*1*/   0,
          1,
          12345,
          -2345,

          // float data
  /*5*/   10.5,
          -10.5,
          12.3456789000e10,
          12.3456789000E-10,
          .5,

          // null data
  /*10*/  NULL,
          null,

          // boolean data
  /*12*/  true,
          false,
          TRUE,
          FALSE,

          // empty data
  /*16*/  "",
          '',

          // string data
  /*18*/  "string",
          'string',

          // object data
  /*20*/  new test(),

          // resource data
  /*21*/  $fp,


  ];

  /* loop through each element of the array to test array_rand() function
   * for different values for 'input' argument */
  $count = 1;
  foreach($values as $value) {
    echo "\n-- Iteration $count --\n";
    var_dump(array_rand($value,$num_req));
    $count++;
  }

  // closing the resource
  fclose($fp);

  echo "Done";
}
