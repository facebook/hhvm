<?hh
/* Prototype  : string wordwrap ( string $str [, int $width [, string $break [, bool $cut]]] )
 * Description: Wraps buffer to selected number of characters using string break char
 * Source code: ext/standard/string.c
*/

/*
 * test wordwrap() by supplying different values for cut argument
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing wordwrap() : usage variations ***\n";
// initialize all required variables
$str = 'testing wordwrap function';
$width = 10;
$break = '<br />\n';


// resource variable
$fp = fopen(__FILE__, "r");

// array with different values
$values =  vec[

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

  // string values
  "string",
  'string',

  // objects
  new stdClass(),

  // empty string
  "",
  '',


];

// loop though each element of the array and check the working of wordwrap()
// when $cut argument is supplied with different values
echo "\n--- Testing wordwrap() by supplying different values for 'cut' argument ---\n";
$counter = 1;
for($index = 0; $index < count($values); $index ++) {
  echo "-- Iteration $counter --\n";
  $cut = $values [$index];

  try { var_dump( wordwrap($str, $width, $break, $cut) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  $counter ++;
}

// close the resource
fclose($fp);

echo "Done\n";
}
