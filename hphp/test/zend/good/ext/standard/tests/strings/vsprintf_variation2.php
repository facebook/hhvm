<?hh
/* Prototype  : string vsprintf(string format, array args)
 * Description: Return a formatted string
 * Source code: ext/standard/formatted_print.c
*/

/*
 * Test vsprintf() when different unexpected values are passed to
 * the '$args' arguments of the function
*/

// declaring a class
class sample
{
  public function __toString() :mixed{
  return "object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing vsprintf() : with unexpected values for args argument ***\n";

// initialising the required variables
$format = '%s';


// Defining resource
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

  // string data
  "string",
  'string',

  // object data
  new sample(),



  // resource data
  $file_handle
];

// loop through each element of the array for args
$counter = 1;
foreach($values as $value) {
  echo "\n-- Iteration $counter --\n";
  var_dump( vsprintf($format,$value) );
  $counter++;
};

// closing the resource
fclose($file_handle);

echo "Done";
}
