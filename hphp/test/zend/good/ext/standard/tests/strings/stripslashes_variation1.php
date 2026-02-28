<?hh
/* Prototype  : string stripslashes ( string $str )
 * Description: Returns an un-quoted string
 * Source code: ext/standard/string.c
*/

/*
 * Test stripslashes() with non-string type argument such as int, float, etc
*/

// declaring a class
class sample  {
  public function __toString() :mixed{
  return "obj\'ct";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing stripslashes() : with non-string type argument ***\n";
// initialize all required variables


// Defining resource
$file_handle = fopen(__FILE__, 'r');

// array with different values
$values =  vec[
          // empty string
          "",
          '',
];


// loop through each element of the array and check the working of stripslashes()
// when $str argument is supplied with different values
echo "\n--- Testing stripslashes() by supplying different values for 'str' argument ---\n";
$counter = 1;
for($index = 0; $index < count($values); $index ++) {
  echo "-- Iteration $counter --\n";
  $str = $values [$index];

  try { var_dump( stripslashes($str) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  $counter ++;
}

// closing the file
fclose($file_handle);

echo "===DONE===\n";
}
