<?hh
/* Prototype  : string wordwrap ( string $str [, int $width [, string $break [, bool $cut]]] )
 * Description: Wraps buffer to selected number of characters using string break char
 * Source code: ext/standard/string.c
*/

/*
 * testing wordwrap() by providing different values for str argument
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing wordwrap() : usage variations ***\n";
// initialize all required variables
$width = 3;
$break = '<br />\n';
$cut = true;

// resource variable
$fp = fopen(__FILE__, "r");


// array with different values
$values =  vec[
  // empty string
  "",
  '',
];

// loop though each element of the array and check the working of wordwrap()
// when $str argument is supplied with different values
echo "\n--- Testing wordwrap() by supplying different values for 'str' argument ---\n";
$counter = 1;
for($index = 0; $index < count($values); $index ++) {
  echo "-- Iteration $counter --\n";
  $str = $values [$index];

  try { var_dump( wordwrap($str) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump( wordwrap($str, $width) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump( wordwrap($str, $width, $break) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  // $cut as false
  $cut = false;
  try { var_dump( wordwrap($str, $width, $break, $cut) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  // $cut as true
  $cut = true;
  try { var_dump( wordwrap($str, $width, $break, $cut) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  $counter ++;
}

// close the resource
fclose($fp);

echo "Done\n";
}
