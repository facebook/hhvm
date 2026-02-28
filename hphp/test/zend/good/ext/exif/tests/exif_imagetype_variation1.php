<?hh

/* Prototype  : int exif_imagetype  ( string $filename  )
 * Description: Determine the type of an image
 * Source code: ext/exif/exif.c
*/

// declaring a class
class sample  {
  public function __toString() :mixed{
  return "obj'ct";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing exif_imagetype() : different types for filename argument ***\n";
// initialize all required variables


// array with different values
$values =  vec[
  // empty string
  "",
  '',
];


// loop through each element of the array and check the working of exif_imagetype()
// when $filename is supplied with different values

echo "\n--- Testing exif_imagetype() by supplying different values for 'filename' argument ---\n";
$counter = 1;
foreach($values as $filename) {
  echo "-- Iteration $counter --\n";
  try { var_dump( exif_imagetype($filename) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $counter ++;
}

echo "Done\n";
echo "===Done===\n";
}
