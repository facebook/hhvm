<?hh
/* Prototype  : string strtok ( string $str, string $token )
 * Description: splits a string (str) into smaller strings (tokens), with each token being delimited by any character from token
 * Source code: ext/standard/string.c
*/

/*
 * Testing strtok() : with first argument as non-string
*/

// declaring a class
class sample  {
  public function __toString() :mixed{
  return "obj-ect";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing strtok() : with first argument as non-string ***\n";
// initialize all required variables
$token = '-';


// Defining resource
$file_handle = fopen(__FILE__, 'r');

// array with different values
$values =  varray [

  // integer values
  0,
  1,
  12345,
  -2345,

  // float values
  10.5,
  -10.5,
  10.1234567e10,
  10.7654321E-10,
  .5,

  // array values
  vec[],
  vec[0],
  vec[1],
  vec[1, 2],
  dict['color' => 'red-color', 'item' => 'pen-color'],

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



  // resource
  $file_handle
];


// loop through each element of the array and check the working of strtok()
// when $str argument is supplied with different values

echo "\n--- Testing strtok() by supplying different values for 'str' argument ---\n";
$counter = 1;
for($index = 0; $index < count($values); $index ++) {
  echo "-- Iteration $counter --\n";
  $str = $values [$index];

  try { var_dump( strtok($str, $token) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  $counter ++;
}

//closing the resource
fclose($file_handle);

echo "Done\n";
}
