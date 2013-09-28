<?php
/* Prototype: array lstat ( string $filename );
   Description: Gives information about a file or symbolic link

   Prototype: array stat ( string $filename );
   Description: Gives information about a file
*/

/* test for stats of link when their names are stored in object and array */

$file_path = dirname(__FILE__);
require "$file_path/file.inc";

$fp = fopen("$file_path/lstat_stat_variation20.tmp", "w");  // temp file
fclose($fp);

// temp link
symlink("$file_path/lstat_stat_variation20.tmp", "$file_path/lstat_stat_variation20_link.tmp");

echo "*** Testing lstat() with linkname stored inside an object/array ***\n";
class names {
  public $var_name;
  public function names($name) {
    $this->var_name = $name;
  }
}

// link name stored in an object
$link_object = new names("$file_path/lstat_stat_variation20_link.tmp");

// link name stored in side an array 
// with default numeric key 
$link_array = array("$file_path/lstat_stat_variation20_link.tmp");

// with string key index
$link_array_with_key = array("linkname" => "$file_path/lstat_stat_variation20_link.tmp");

echo "\n-- Testing lstat() on link name stored inside an object --\n";
var_dump( lstat($link_object->var_name) );

echo "\n-- Testing stat() on link name stored inside an array --\n";
var_dump( stat($link_array[0]) ); // with default numeric index
var_dump( stat($link_array_with_key["linkname"]) ); // with string key
var_dump( stat($link_array_with_key['linkname']) );

echo "\n--- Done ---";
?>
<?php
$file_path = dirname(__FILE__);
unlink("$file_path/lstat_stat_variation20_link.tmp");
unlink("$file_path/lstat_stat_variation20.tmp");
?>