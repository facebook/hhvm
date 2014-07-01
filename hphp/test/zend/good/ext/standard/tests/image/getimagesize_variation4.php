<?php
/* Prototype  : array getimagesize(string imagefile [, array info])
 * Description: Get the size of an image as 4-element array 
 * Source code: ext/standard/image.c
 */

echo "*** Testing getimagesize() : variation ***\n";

var_dump( getimagesize(dirname(__FILE__)."/test13pix.swf", $info) );
var_dump( $info );
?>
===DONE===