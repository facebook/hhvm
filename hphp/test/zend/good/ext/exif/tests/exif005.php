<?php
/* Do not change this test it is a README.TESTING example.
 * test5.jpg is a 1*1 image that contains an Exif section with ifd = 00000009h
 */
$image  = exif_read_data(dirname(__FILE__).'/test5.jpg','',true,false);
var_dump($image['IFD0']);
?>