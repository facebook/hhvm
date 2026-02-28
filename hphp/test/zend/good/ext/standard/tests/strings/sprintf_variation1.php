<?hh
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string
 * Source code: ext/standard/formatted_print.c
*/

/*
* Testing sprintf() : with different unexpected values for format argument other than the strings
*/

// declaring class
class sample
{
  public function __toString() :mixed{
    return "Object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing sprintf() : with unexpected values for format argument ***\n";

// initialing required variables
$arg1 = "second arg";
$arg2 = "third arg";


// creating a file resource
$file_handle = fopen(__FILE__, 'r');

//array of values to iterate over
$values = vec[

      // int data
      0,
      1,
      12345,
      -2345,

      // float data
      10.5,
      -10.5,
      10.1234567e10,
      10.7654321E-10,
      .5,

      // array data
      'Array',
      'Array',
      'Array',
      'Array',
      'Array',

      // null data
      NULL,
      null,

      // boolean data
      true,
      false,
      TRUE,
      FALSE,

      // empty data
      "",
      '',

      // object data
      new sample(),



      // resource data
      $file_handle
];

// loop through each element of the array for format

$count = 1;
foreach($values as $value) {
  echo "\n-- Iteration $count --\n";

  // with default argument
  var_dump( sprintf($value) );

  // with two arguments
  var_dump( sprintf($value, $arg1) );

  // with three arguments
  var_dump( sprintf($value, $arg1, $arg2) );

  $count++;
};

// close the resource
fclose($file_handle);

echo "Done";
}
