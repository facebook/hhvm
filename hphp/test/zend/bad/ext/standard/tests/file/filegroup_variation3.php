<?php
/* Prototype: int filegroup ( string $filename )
 * Description: Returns the group ID of the file, or FALSE in case of an error.
 */

/* Passing file names with different notations, using slashes, wild-card chars */

$file_path = dirname(__FILE__);

echo "*** Testing filegroup() with different notations of file names ***\n";
$dir_name = $file_path."/filegroup_variation3";
mkdir($dir_name);
$file_handle = fopen($dir_name."/filegroup_variation3.tmp", "w");
fclose($file_handle);

$files_arr = array(
  "/filegroup_variation3/filegroup_variation3.tmp",

  /* Testing a file trailing slash */
  "/filegroup_variation3/filegroup_variation3.tmp/",

  /* Testing file with double slashes */
  "/filegroup_variation3//filegroup_variation3.tmp",
  "//filegroup_variation3//filegroup_variation3.tmp",
  "/filegroup_variation3/*.tmp",
  "filegroup_variation3/filegroup*.tmp", 

  /* Testing Binary safe */
  "/filegroup_variation3/filegroup_variation3.tmp".chr(0),
  "/filegroup_variation3/filegroup_variation3.tmp\0"
);

$count = 1;
/* loop through to test each element in the above array */
foreach($files_arr as $file) {
  echo "- Iteration $count -\n";
  var_dump( filegroup( $file_path."/".$file ) );
  clearstatcache();
  $count++;
}

echo "\n*** Done ***";
?>
<?php
$file_path = dirname(__FILE__);
$dir_name = $file_path."/filegroup_variation3";
unlink($dir_name."/filegroup_variation3.tmp");
rmdir($dir_name);
?>