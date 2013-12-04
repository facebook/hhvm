<?php
/*
  test4.jpg is a 1*1 image that contains Exif tags written by WindowsXP
*/
$image  = exif_read_data(dirname(__FILE__).'/test4.jpg','',true,false);
echo var_dump($image['WINXP']);
?>