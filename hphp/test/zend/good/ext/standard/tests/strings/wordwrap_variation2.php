<?hh
/* Prototype  : string wordwrap ( string $str [, int $width [, string $break [, bool $cut]]] )
 * Description: Wraps buffer to selected number of characters using string break char
 * Source code: ext/standard/string.c
*/

/*
 * test wordwrap by passing different values for width argument */
<<__EntryPoint>> function main(): void {
echo "*** Testing wordwrap() : usage variations ***\n";
// initialize all required variables
$str = 'testing wordwrap function';
$break = '<br />\n';
$cut = true;

// resource var
$fp = fopen(__FILE__, "r");



// array with different values as width
$values =  vec[
  // zerovalue for width
  0,

  // -ve value for width
  -1,
  -10,
];


// loop though each element of the array and check the working of wordwrap()
// when $width argument is supplied with different values
echo "\n--- Testing wordwrap() by supplying different values for 'width' argument ---\n";
$counter = 1;
for($index = 0; $index < count($values); $index ++) {
  echo "-- Iteration $counter --\n";
  $width = $values [$index];

  try { var_dump( wordwrap($str, $width) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump( wordwrap($str, $width, $break) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  // cut as false
  $cut = false;
  try { var_dump( wordwrap($str, $width, $break, $cut) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  // cut as true
  $cut = true;
  try { var_dump( wordwrap($str, $width, $break, $cut) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  $counter ++;
}

// close the resource
fclose($fp);

echo "Done\n";
}
