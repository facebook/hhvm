<?php
/* Prototype  : array getimagesize(string imagefile [, array info])
 * Description: Get the size of an image as 4-element array 
 * Source code: ext/standard/image.c
 */


$file_types_array = array (
    //File containing text string
	"File with text data" => "test.txt",

	//File containing forcibly corrupted bmp image
	"File with corrupted BMP data" => "200x100_unknown.unknown",

    //File which doesn't exist
     "Non-existent file" => "nofile.ext",

    //File having no data
    "Empty File" => "blank_file.bmp"
);

echo "*** Testing getimagesize() : variation ***\n";

//loop through each element of the array for filename
foreach($file_types_array as $key => $filename) {
      echo "\n-- $key ($filename) --\n";
      var_dump( getimagesize(dirname(__FILE__)."/$filename" ) );
      var_dump( getimagesize(dirname(__FILE__)."/$filename", $info) );
      var_dump( $info );
};
?>
===DONE===