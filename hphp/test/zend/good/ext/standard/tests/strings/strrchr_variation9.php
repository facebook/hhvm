<?hh
/* Prototype  : string strrchr(string $haystack, string $needle);
 * Description: Finds the last occurrence of a character in a string.
 * Source code: ext/standard/string.c
*/

/* Test strrchr() function with unexpected inputs for haystack
 *  and expected type for 'needle'
*/

// declaring a class
class sample  {
  public function __toString() :mixed{
    return "object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing strrchr() function: with unexpected inputs for haystack ***\n";


//getting the resource
$file_handle = fopen(__FILE__, "r");

// array with different values
$haystacks =  vec[
  // empty string
  "",
  '',
];

$needles =  vec[
  //integer numeric strings
  "0",
  "1",
  "2",
  "-2",

  //float numeric strings
  "10.5",
  "-10.5",
  "10.5e10",
  "10.6E-10",
  ".5",

  //regular strings
  "array",
  "a",
  "r",
  "y",
  "ay",
  "true",
  "false",
  "TRUE",
  "FALSE",
  "NULL",
  "null",
  "object",

  //empty string
  "",
  '',

  //resource variable in string form
  "\$file_handle",

];

// loop through each element of the array and check the working of strrchr()
$count = 1;
for($index = 0; $index < count($haystacks); $index++) {
  echo "-- Iteration $count --\n";
  try { var_dump( strrchr($haystacks[$index], $needles[$index]) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $count ++;
}

fclose($file_handle);  //closing the file handle

echo "*** Done ***";
}
