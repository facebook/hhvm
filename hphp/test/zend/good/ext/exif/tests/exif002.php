<?php
/*
  test1.jpg is a 1*1 image that does not contain any Exif/Comment information
  test2.jpg is the same image but contains Exif/Comment information and a
            copy of test1.jpg as a thumbnail.
*/
$infile = dirname(__FILE__).'/test1.jpg';
echo md5_file($infile).'_'.filesize($infile);
$thumb = exif_thumbnail(dirname(__FILE__).'/test2.jpg');
echo " == ";
echo md5($thumb).'_'.strlen($thumb);
echo "\n";
?>