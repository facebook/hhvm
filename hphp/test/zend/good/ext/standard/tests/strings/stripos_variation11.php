<?hh
/* Prototype  : int stripos ( string $haystack, string $needle [, int $offset] );
 * Description: Find position of first occurrence of a case-insensitive string
 * Source code: ext/standard/string.c
*/

/* Test stripos() function with unexpected inputs for 'haystack' and 'needle' arguments */

// defining a class
class sample  {
  public function __toString() :mixed{
    return "object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing stripos() function with unexpected values for haystack and needle ***\n";


//getting the resource
$file_handle = fopen(__FILE__, "r");

// array with different values
$values =  vec[
  // empty string
  "",
  '',
];


// loop through each element of the array and check the working of stripos()
$counter = 1;
for($index = 0; $index < count($values); $index ++) {
  echo "-- Iteration $counter --\n";
  $haystack = $values[$index];
  try { var_dump( stripos($values[$index], $values[$index]) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump( stripos($values[$index], $values[$index], 1) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $counter ++;
}

echo "*** Done ***";
}
