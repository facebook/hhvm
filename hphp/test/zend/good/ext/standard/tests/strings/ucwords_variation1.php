<?hh
/* Prototype  : string ucwords ( string $str )
 * Description: Uppercase the first character of each word in a string
 * Source code: ext/standard/string.c
*/

/*
 * Test ucwords() by passing different values including scalar and non scalar values
*/

class my
{
  function __toString() :mixed{
    return "myString";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing ucwords() : usage variations ***\n";
// initialize all required variables


// array with different values
$values =  vec[
  // empty string
  "",
  '',

  // hex in string
  "0x123",
  '0x123',
  "0xFF12",
  "-0xFF12",
];

// loop through each element of the array and check the working of ucwords()
// when $str argument is supplied with different values
echo "\n--- Testing ucwords() by supplying different values for 'str' argument ---\n";
$counter = 1;
for($index = 0; $index < count($values); $index ++) {
  echo "-- Iteration $counter --\n";
  $str = $values [$index];

  try { var_dump( ucwords($str) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  $counter ++;
}

echo "Done\n";
}
