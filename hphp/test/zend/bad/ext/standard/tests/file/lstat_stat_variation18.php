<?php
/* Prototype: array lstat ( string $filename );
   Description: Gives information about a file or symbolic link

   Prototype: array stat ( string $filename );
   Description: Gives information about a file
*/

/* test for stats of dir/file when their names are stored in objects */

$file_path = dirname(__FILE__);
require "$file_path/file.inc";


/* create temp file and directory */
mkdir("$file_path/lstat_stat_variation18/");  // temp dir
$fp = fopen("$file_path/lstat_stat_variation18.tmp", "w");  // temp file
fclose($fp);

echo "*** Testing stat() with filename & directory name stored inside an object ***\n";

class names {
  public $var_name;
  public function names($name) {
    $this->var_name = $name;
  }
}

// directory name stored in an object
$dir_name = new names("$file_path/lstat_stat_variation18");

// file name stored in an object 
$file_name = new names("$file_path/lstat_stat_variation18.tmp");

echo "\n-- Testing stat() on filename stored inside an object --\n";
// dump the stat returned value 
var_dump( stat($file_name->var_name) );

echo "\n-- Testing stat() on directory name stored inside an object --\n";
// dump the stat returned value 
var_dump( stat($dir_name->var_name) );

echo "\n--- Done ---";
?>
<?php
$file_path = dirname(__FILE__);
unlink("$file_path/lstat_stat_variation18.tmp");
rmdir("$file_path/lstat_stat_variation18");
?>