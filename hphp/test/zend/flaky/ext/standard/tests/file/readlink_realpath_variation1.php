<?php
/* Prototype: string readlink ( string $path );
   Description: Returns the target of a symbolic link

   Prototype: string realpath ( string $path );
   Description: Returns canonicalized absolute pathname
*/

echo "*** Testing readlink() and realpath() : usage variations ***\n";
$name_prefix = dirname(__FILE__);
$filename = "$name_prefix/readlink_realpath_variation1/home/tests/link/readlink_realpath_variation1.tmp";
mkdir("$name_prefix/readlink_realpath_variation1/home/tests/link/", 0777, true);

echo "\n*** Testing readlink() and realpath() with linkname stored inside a object ***\n";
// create a temp file
$file_handle = fopen($filename, "w");
fclose($file_handle);

// creating object with members as linkname
class object_temp {
  public $linkname;
  function object_temp($link) {
    $this->linkname = $link;
  }
}
$obj1 = new object_temp("$name_prefix/readlink_realpath_variation1/../././readlink_realpath_variation1/home/readlink_realpath_variation1_link.tmp");
$obj2 = new object_temp("$name_prefix/readlink_realpath_variation1/home/../..///readlink_realpath_variation1_link.tmp");

echo "\n-- Testing readlink() and realpath() with softlink, linkname stored inside an object --\n";
// creating the links 
var_dump( symlink($filename, $obj1->linkname) );  
var_dump( readlink($obj1->linkname) ); 
var_dump( realpath($obj1->linkname) );
var_dump( symlink($filename, $obj2->linkname) ); 
var_dump( readlink($obj2->linkname) );
var_dump( realpath($obj2->linkname) );

// deleting the link
unlink($obj1->linkname);
unlink($obj2->linkname);  

echo "\n-- Testing readlink() and realpath() with hardlink, linkname stored inside an object --\n";
// creating hard links
var_dump( link($filename, $obj1->linkname) );  
var_dump( readlink($obj1->linkname) );   // invalid because readlink doesn't work with hardlink
var_dump( realpath($obj1->linkname) );
var_dump( link($filename, $obj2->linkname) );  
var_dump( readlink($obj2->linkname) );   // invalid because readlink doesn't work with hardlink
var_dump( realpath($obj2->linkname) );

// delete the links 
unlink($obj1->linkname);
unlink($obj2->linkname);  

echo "Done\n";
?>
<?php
$name_prefix = dirname(__FILE__)."/readlink_realpath_variation1";
unlink("$name_prefix/home/tests/link/readlink_realpath_variation1.tmp");
rmdir("$name_prefix/home/tests/link/");
rmdir("$name_prefix/home/tests/");
rmdir("$name_prefix/home/");
rmdir("$name_prefix/");
?>