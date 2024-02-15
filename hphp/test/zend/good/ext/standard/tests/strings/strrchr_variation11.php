<?hh
/* Prototype  : string strrchr(string $haystack, string $needle);
 * Description: Finds the last occurrence of a character in a string.
 * Source code: ext/standard/string.c
*/

/* Test strrchr() function with unexpected inputs for haystack and needle */

// declaring a class
class sample  {
  public function __toString() :mixed{
    return "object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing strrchr() function: with unexpected inputs for haystack and needle ***\n";


//getting the resource
$file_handle = fopen(__FILE__, "r");

// array with different values
$values =  vec[
  // empty string
  "",
  '',
];


// loop through each element of the array and check the working of strrchr()
$counter = 1;
for($index = 0; $index < count($values); $index ++) {
  echo "-- Iteration $counter --\n";
  try { var_dump( strrchr($values[$index], $values[$index]) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $counter ++;
}

fclose($file_handle);  //closing the file handle

echo "*** Done ***";
}
