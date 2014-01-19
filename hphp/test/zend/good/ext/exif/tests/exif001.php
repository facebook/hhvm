<?php
/*
  test1.jpg is a 1*1 image that does not contain any Exif/Comment information
  test2.jpg is the same image but contains Exif/Comment information and a
            copy of test1.jpg as a thumbnail.
*/
var_dump(exif_read_data(dirname(__FILE__).'/test2.jpg','',true,false));
?>