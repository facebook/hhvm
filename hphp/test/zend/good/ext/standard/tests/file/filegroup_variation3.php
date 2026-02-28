<?hh
/* Prototype: int filegroup ( string $filename )
 * Description: Returns the group ID of the file, or FALSE in case of an error.
 */

/* Passing file names with different notations, using slashes, wild-card chars */
<<__EntryPoint>> function main(): void {


echo "*** Testing filegroup() with different notations of file names ***\n";
$dir_name = sys_get_temp_dir().'/'.'filegroup_variation3';
mkdir($dir_name);
$file_handle = fopen($dir_name."/filegroup_variation3.tmp", "w");
fclose($file_handle);

$files_arr = vec[
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
];

$count = 1;
/* loop through to test each element in the above array */
foreach($files_arr as $file) {
  echo "- Iteration $count -\n";
  var_dump( filegroup(sys_get_temp_dir().'/'.$file)) ;
  clearstatcache();
  $count++;
}

echo "\n*** Done ***";
error_reporting(0);
unlink($dir_name."/filegroup_variation3.tmp");
rmdir($dir_name);
}
