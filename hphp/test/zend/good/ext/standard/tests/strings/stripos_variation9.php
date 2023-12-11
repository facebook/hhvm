<?hh
/* Prototype  : int stripos ( string $haystack, string $needle [, int $offset] );
 * Description: Find position of first occurrence of a case-insensitive string
 * Source code: ext/standard/string.c
*/

/* Test stripos() function with unexpected inputs for haystack argument */

// defining a class
class sample  {
  public function __toString() :mixed{
    return "object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing stripos() function with unexpected values for haystack ***\n";


//getting the resource
$file_handle = fopen(__FILE__, "r");

// array with different values
$haystacks =  varray [

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

  // resource
  $file_handle,
];

$needle = "heredoc 0 1 2 -2 10.5 -10.5 10.5e10 10.6E-10 .5 array true false object \"\" null Resource";

// loop through each element of the array and check the working of stripos()
$counter = 1;
for($index = 0; $index < count($haystacks); $index ++) {
  echo "\n-- Iteration $counter --\n";
  try { var_dump( stripos($haystacks[$index], $needle) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $counter ++;
}

fclose($file_handle);  //closing the file handle

echo "*** Done ***";
}
