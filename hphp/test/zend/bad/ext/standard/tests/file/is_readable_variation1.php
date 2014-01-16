<?php
/* Prototype: bool is_readable ( string $filename );
   Description: Tells whether the filename is readable.
*/

/* test is_readable() with file having different filepath notation */

require dirname(__FILE__).'/file.inc';
echo "*** Testing is_readable(): usage variations ***\n";

$file_path = dirname(__FILE__);
mkdir("$file_path/is_readable_variation1");

// create a new temporary file
$fp = fopen("$file_path/is_readable_variation1/bar.tmp", "w");
fclose($fp);

/* array of files to be tested if they are readable by using
   is_readable() function */
$files_arr = array(
  "$file_path/is_readable_variation1/bar.tmp",

  /* Testing a file trailing slash */
  "$file_path/is_readable_variation1/bar.tmp/",

  /* Testing file with double slashes */
  "$file_path/is_readable_variation1//bar.tmp",
  "$file_path//is_readable_variation1//bar.tmp",
  "$file_path/is_readable_variation1/*.tmp",
  "$file_path/is_readable_variation1/b*.tmp", 

  /* Testing Binary safe */
  "$file_path/is_readable_variation1".chr(0)."bar.tmp",
  "$file_path".chr(0)."is_readable_variation1/bar.tmp",
  "$file_path".chr(0)."is_readable_variation1/bar.tmp",
  
  /* Testing directories */
  ".",  // current directory, exp: bool(true)
  "$file_path/is_readable_variation1"  // temp directory, exp: bool(true)
);
$counter = 1;
/* loop through to test each element in the above array 
   is a writable file */
foreach($files_arr as $file) {
  echo "-- Iteration $counter --\n";
  var_dump( is_readable($file) );
  $counter++;
  clearstatcache();
}

echo "Done\n";
?>
<?php
unlink(dirname(__FILE__)."/is_readable_variation1/bar.tmp");
rmdir(dirname(__FILE__)."/is_readable_variation1/");
?>