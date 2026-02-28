<?hh
/* Prototype  : string htmlspecialchars_decode(string $string [, int $quote_style])
 * Description: Convert special HTML entities back to characters
 * Source code: ext/standard/html.c
*/

/*
 * testing htmlspecialchars_decode() with unexpected input values for $string argument
*/

//get a class
class classA
{
  function __toString() :mixed{
    return "ClassAObject";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing htmlspecialchars_decode() : usage variations ***\n";

//get a resource variable
$file_handle=fopen(__FILE__, "r");


//array of values to iterate over
$values = vec[
      // empty data
      "",
      '',
];

// loop through each element of the array for string
$iterator = 1;
foreach($values as $value) {
      echo "-- Iterator $iterator --\n";
      try { var_dump( htmlspecialchars_decode($value) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
      $iterator++;
};

// close the file resource used
fclose($file_handle);

echo "===DONE===\n";
}
