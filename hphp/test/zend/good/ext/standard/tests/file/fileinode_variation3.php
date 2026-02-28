<?hh
/*
 * Prototype: int fileinode ( string $filename );
 * Description: Returns the inode number of the file, or FALSE in case of an error.
 */

/* Passing file names with different notations, using slashes, wild-card chars */
<<__EntryPoint>> function main(): void {


echo "*** Testing fileinode() with different notations of file names ***\n";
$dir_name = sys_get_temp_dir().'/'.'fileinode_variation3';
mkdir($dir_name);
$file_handle = fopen($dir_name."/fileinode_variation3.tmp", "w");
fclose($file_handle);

$files_arr = vec[
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
];

$count = 1;
/* loop through to test each element in the above array */
foreach($files_arr as $file) {
  echo "- Iteration $count -\n";
  var_dump( fileinode(sys_get_temp_dir().'/'.$file)) ;
  clearstatcache();
  $count++;
}

echo "\n*** Done ***";

unlink($dir_name."/fileinode_variation3.tmp");
rmdir($dir_name);
}
