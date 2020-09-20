<?hh

/* Prototype  : int exif_imagetype  ( string $filename  )
 * Description: Determine the type of an image
 * Source code: ext/exif/exif.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing exif_imagetype() : error conditions ***\n";

echo "\n-- Testing exif_imagetype() function with no arguments --\n";
try { var_dump( exif_imagetype() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing exif_imagetype() function with more than expected no. of arguments --\n";
$extra_arg = 10;
try { var_dump( exif_imagetype(dirname(__FILE__).'/test2.jpg', $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing exif_imagetype() function with an unknown file  --\n";
var_dump( exif_imagetype(dirname(__FILE__).'/foo.jpg') );
echo "===Done===";
}
