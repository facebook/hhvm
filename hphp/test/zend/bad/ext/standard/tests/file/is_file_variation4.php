<?php
/* Prototype: bool is_file ( string $filename );
   Description: Tells whether the filename is a regular file
     Returns TRUE if the filename exists and is a regular file
*/

/* Passing file names with different notations, using slashes, wild-card chars */

$file_path = dirname(__FILE__);

echo "*** Testing is_file() with different notations of file names ***\n";
$dir_name = $file_path."/is_file_variation4";
mkdir($dir_name);
$file_handle = fopen($dir_name."/is_file_variation4.tmp", "w");
fclose($file_handle);

$files_arr = array(
  "/is_file_variation4/is_file_variation4.tmp",

  /* Testing a file trailing slash */
  "/is_file_variation4/is_file_variation4.tmp/",

  /* Testing file with double slashes */
  "/is_file_variation4//is_file_variation4.tmp",
  "//is_file_variation4//is_file_variation4.tmp",
  "/is_file_variation4/*.tmp",
  "is_file_variation4/is_file*.tmp", 

  /* Testing Binary safe */
  "/is_file_variation4/is_file_variation4.tmp".chr(0),
  "/is_file_variation4/is_file_variation4.tmp\0"
);

$count = 1;
/* loop through to test each element in the above array */
foreach($files_arr as $file) {
  echo "- Iteration $count -\n";
  var_dump( is_file( $file_path."/".$file ) );
  clearstatcache();
  $count++;
}

echo "\n*** Done ***";
?>
<?php
$file_path = dirname(__FILE__);
$dir_name = $file_path."/is_file_variation4";
unlink($dir_name."/is_file_variation4.tmp");
rmdir($dir_name);
?>