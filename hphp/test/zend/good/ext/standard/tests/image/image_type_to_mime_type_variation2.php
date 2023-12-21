<?hh
/* Prototype  : string image_type_to_mime_type(int imagetype)
 * Description: Get Mime-Type for image-type returned by getimagesize, exif_read_data, exif_thumbnail, exif_imagetype 
 * Source code: ext/standard/image.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing image_type_to_mime_type() : usage variations ***\n";

error_reporting(E_ALL ^ E_NOTICE);
$values =  vec[
  //Decimal values
  0,
  1,
  12345,
  -12345,
  
  //Octal values
  02,
  010,
  030071,
  -030071,
  
  //Hexadecimal values
  0x0,
  0x1,
  0xABCD,
  -0xABCD
];

// loop through each element of the array for imagetype
$iterator = 1;
foreach($values as $value) {
      echo "\n-- Iteration $iterator --\n";
      var_dump( image_type_to_mime_type($value) );
      $iterator++;
};
echo "===DONE===\n";
}
