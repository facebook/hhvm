<?php
/* 
Prototype: int fileinode ( string $filename );
Description: Returns the inode number of the file, or FALSE in case of an error.
*/

/* Passing file names with different notations, using slashes, wild-card chars */

$file_path = dirname(__FILE__);

echo "*** Testing fileinode() with different notations of file names ***\n";
$dir_name = $file_path."/fileinode_variation3";
mkdir($dir_name);
$file_handle = fopen($dir_name."/fileinode_variation3.tmp", "w");
fclose($file_handle);

$files_arr = array(
  "/fileinode_variation3/fileinode_variation3.tmp",

  /* Testing a file trailing slash */
  "/fileinode_variation3/fileinode_variation3.tmp/",

  /* Testing file with double slashes */
  "/fileinode_variation3//fileinode_variation3.tmp",
  "//fileinode_variation3//fileinode_variation3.tmp",
  "/fileinode_variation3/*.tmp",
  "fileinode_variation3/fileinode*.tmp", 

  /* Testing Binary safe */
  "/fileinode_variation3/fileinode_variation3.tmp".chr(0),
  "/fileinode_variation3/fileinode_variation3.tmp\0"
);

$count = 1;
/* loop through to test each element in the above array */
foreach($files_arr as $file) {
  echo "- Iteration $count -\n";
  var_dump( fileinode( $file_path."/".$file ) );
  clearstatcache();
  $count++;
}

echo "\n*** Done ***";
?>
<?php error_reporting(0); ?>
<?php
$file_path = dirname(__FILE__);
$dir_name = $file_path."/fileinode_variation3";
unlink($dir_name."/fileinode_variation3.tmp");
rmdir($dir_name);
?>