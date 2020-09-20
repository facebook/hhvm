<?hh
/* Prototype  : proto string image_type_to_mime_type(int imagetype)
 * Description: Get Mime-Type for image-type returned by getimagesize, exif_read_data, exif_thumbnail, exif_imagetype 
 * Source code: ext/standard/image.c
 */
<<__EntryPoint>> function main(): void {
$imagetype = IMAGETYPE_GIF;
$extra_arg = 10;
echo "*** Testing image_type_to_mime_type() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing image_type_to_mime_type() function with Zero arguments --\n";
try { var_dump( image_type_to_mime_type() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test image_type_to_mime_type with one more than the expected number of arguments
echo "\n-- Testing image_type_to_mime_type() function with more than expected no. of arguments --\n";
try { var_dump( image_type_to_mime_type($imagetype, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
}
