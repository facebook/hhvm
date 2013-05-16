<?php
/* Prototype  : proto array getimagesize(string imagefile [, array info])
 * Description: Get the size of an image as 4-element array 
 * Source code: ext/standard/image.c
 * Alias to functions: 
 */

echo "*** Testing getimagesize() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing getimagesize() function with Zero arguments --\n";
var_dump( getimagesize() );

//Test getimagesize with one more than the expected number of arguments
echo "\n-- Testing getimagesize() function with more than expected no. of arguments --\n";
$imagefile = 'string_val';
$info = array(1, 2);
$extra_arg = 10;
var_dump( getimagesize($imagefile, $info, $extra_arg) );

?>
===DONE===