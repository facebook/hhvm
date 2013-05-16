<?php
/* Prototype  : proto array getimagesize(string imagefile [, array info])
 * Description: Get the size of an image as 4-element array 
 * Source code: ext/standard/image.c
 * Alias to functions: 
 */

echo "*** Testing getimagesize() : tiff_mm format ***\n";
var_dump(getimagesize(dirname(__FILE__) . "/2x2mm.tif", $arr));
var_dump($arr);

?>
===DONE===