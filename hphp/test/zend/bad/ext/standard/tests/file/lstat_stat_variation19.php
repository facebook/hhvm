<?php
/* Prototype: array lstat ( string $filename );
   Description: Gives information about a file or symbolic link

   Prototype: array stat ( string $filename );
   Description: Gives information about a file
*/

/* test for stats of dir/file when their names are stored in an array */

$file_path = dirname(__FILE__);
require "$file_path/file.inc";


/* create temp file, link and directory */
@rmdir("$file_path/lstat_stat_variation19");  // ensure that dir doesn't exists
mkdir("$file_path/lstat_stat_variation19");  // temp dir

$fp = fopen("$file_path/lstat_stat_variation19.tmp", "w");  // temp file
fclose($fp);

echo "*** Testing stat() with filename & directory name stored inside an array ***\n";

// array with default numeric index
$names = array(
  "$file_path/lstat_stat_variation19.tmp", 
  "$file_path/lstat_stat_variation19"
);

//array with string key index
$names_with_key = array (
  'file' => "$file_path/lstat_stat_variation19.tmp",
  "dir" => "$file_path/lstat_stat_variation19"
);

echo "\n-- Testing stat() on filename stored inside an array --\n";
var_dump( stat($names[0]) ); // values stored with numeric index
var_dump( stat($names_with_key['file']) ); // value stored with string key

echo "\n-- Testing stat() on dir name stored inside an array --\n";
var_dump( stat($names[1]) ); // values stored with numeric index
var_dump( stat($names_with_key["dir"]) ); // value stored with string key

echo "\n--- Done ---";
?>
<?php
$file_path = dirname(__FILE__);
unlink("$file_path/lstat_stat_variation19.tmp");
rmdir("$file_path/lstat_stat_variation19");
?>