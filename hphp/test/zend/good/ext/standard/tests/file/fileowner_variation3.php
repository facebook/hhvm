<?hh
/* Prototype: int fileowner ( string $filename )
 * Description: Returns the user ID of the owner of the file, or
 *              FALSE in case of an error.
 */

/* Passing file names with different notations, using slashes, wild-card chars */
<<__EntryPoint>> function main(): void {


echo "*** Testing fileowner() with different notations of file names ***\n";
$dir_name = sys_get_temp_dir().'/'.'fileowner_variation3';
mkdir($dir_name);
$file_handle = fopen($dir_name."/fileowner_variation3.tmp", "w");
fclose($file_handle);

$files_arr = vec[
  "/fileowner_variation3/fileowner_variation3.tmp",

  /* Testing a file trailing slash */
  "/fileowner_variation3/fileowner_variation3.tmp/",

  /* Testing file with double slashes */
  "/fileowner_variation3//fileowner_variation3.tmp",
  "//fileowner_variation3//fileowner_variation3.tmp",
  "/fileowner_variation3/*.tmp",
  "fileowner_variation3/fileowner*.tmp", 

  /* Testing Binary safe */
  "/fileowner_variation3/fileowner_variation3.tmp".chr(0),
  "/fileowner_variation3/fileowner_variation3.tmp\0"
];

$count = 1;
/* loop through to test each element in the above array */
foreach($files_arr as $file) {
  echo "- Iteration $count -\n";
  var_dump( fileowner(sys_get_temp_dir().'/'.$file)) ;
  clearstatcache();
  $count++;
}

echo "\n*** Done ***";

unlink($dir_name."/fileowner_variation3.tmp");
rmdir($dir_name);
}
