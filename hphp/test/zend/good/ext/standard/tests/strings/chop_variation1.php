<?hh
/* Prototype  : string chop ( string $str [, string $charlist] )
 * Description: Strip whitespace (or other characters) from the end of a string
 * Source code: ext/standard/string.c
*/

/*
 * Testing chop() : with different unexpected values for $str argument passed to the function
*/

class sample  {
  public function __toString() :mixed{
    return " @#$%Object @#$%";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing chop() : with unexpected values for str argument ***\n";
// initialize all required variables

$charlist = " @#$%1234567890";

$sample_obj = new sample;

// creating a file resource
$file_handle = fopen(__FILE__, 'r');

// array with different values
$values =  vec[
  // empty string
  "",
  '',
];


// loop through each element of the array and check the working of chop()
// when $str argument is supplied with different values

echo "\n--- Testing chop() by supplying different values for 'str' argument ---\n";
$counter = 1;
for($index = 0; $index < count($values); $index ++) {
  echo "-- Iteration $counter --\n";
  $str = $values [$index];

  try { var_dump( chop($str) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump( chop($str, $charlist) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  $counter ++;
}

// closing the resource
fclose( $file_handle);

echo "Done\n";
}
