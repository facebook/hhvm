<?php
/* Prototype  : proto array getimagesize(string imagefile [, array info])
 * Description: Get the size of an image as 4-element array
 * Source code: ext/standard/image.c
 * Alias to functions:
 */

echo "*** Testing getimagesize() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing getimagesize() function with Zero arguments --\n";
try { var_dump( getimagesize() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test getimagesize with one more than the expected number of arguments
echo "\n-- Testing getimagesize() function with more than expected no. of arguments --\n";
$imagefile = 'string_val';
$info = array(1, 2);
$extra_arg = 10;
try { var_dump( getimagesize($imagefile, &$info, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
