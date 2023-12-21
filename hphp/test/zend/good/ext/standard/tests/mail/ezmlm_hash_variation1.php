<?hh

/* Prototype  : array explode  ( string $delimiter  , string $string  [, int $limit  ] )
 * Description: Split a string by string.
 * Source code: ext/standard/string.c
*/

//defining a class
class sample  {
  public function __toString() :mixed{
    return "sample object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing explode() function: with unexpected inputs for 'delimiter' argument ***\n";

//getting the resource
$file_handle = fopen(__FILE__, "r");

// array with different values for $delimiter
$delimiters =  vec[

  // integer values
  0,
  1,
  255,
  256,
  PHP_INT_MAX,
  -PHP_INT_MAX,

  // float values
  10.5,
  -20.5,
  10.1234567e10,

  // array values
  vec[],
  vec[0],
  vec[1, 2],

  // boolean values
  true,
  false,
  TRUE,
  FALSE,

  // null values
  NULL,
  null,

  // objects
  new sample(),

  // resource
  $file_handle
];

// loop through with each element of the $delimiters array to test explode() function
$count = 1;
$string = "piece1 piece2 piece3 piece4 piece5 piece6";
$limit = 5;
foreach($delimiters as $delimiter) {
  echo "-- Iteration $count --\n";
  try { var_dump( explode($delimiter, $string, $limit) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $count ++;
}

fclose($file_handle);  //closing the file handle
echo "===Done===";
}
