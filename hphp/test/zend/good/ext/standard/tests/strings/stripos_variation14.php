<?hh
/* Prototype  : int stripos ( string $haystack, string $needle [, int $offset] );
 * Description: Find position of first occurrence of a case-insensitive string
 * Source code: ext/standard/string.c
*/

/* Test stripos() function with unexpected inputs for 'offset' argument */

// defining a class
class sample  {
  public function __toString() :mixed{
    return "object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing stripos() function with unexpected values for offset ***\n";


//getting the resource
$file_handle = fopen(__FILE__, "r");

//definition of input args
$haystack = "hello world";
$needle = "world";

// array with different values
$offsets =  varray [

  // float values
  1.5,
  -1.5,
  1.5e10,
  1.6E-10,
  .5,

  // array values
  vec[],
  vec[0],
  vec[1],
  vec[1, 2],
  dict['color' => 'red', 'item' => 'pen'],

  // boolean values
  true,
  false,
  TRUE,
  FALSE,

  // objects
  new sample(),

  // empty string
  "",
  '',

  // null vlaues
  NULL,
  null,

  //resource
  $file_handle,
];


// loop through each element of the array and check the working of stripos()
$counter = 1;
for($index = 0; $index < count($offsets); $index ++) {
  echo "-- Iteration $counter --\n";
  try { var_dump( stripos($haystack, $needle, $offsets[$index]) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $counter ++;
}

echo "*** Done ***";
}
