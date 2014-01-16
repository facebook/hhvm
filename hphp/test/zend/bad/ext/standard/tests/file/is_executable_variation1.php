<?php
/* Prototype: bool is_executable ( string $filename );
   Description: Tells whether the filename is executable
*/

/* test is_executable() with file having different filepath notation */

require dirname(__FILE__).'/file.inc';
echo "*** Testing is_executable(): usage variations ***\n";

$file_path = dirname(__FILE__);
mkdir("$file_path/is_executable_variation1");

// create a new temporary file
$fp = fopen("$file_path/is_executable_variation1/bar.tmp", "w");
fclose($fp);

/* array of files checked to see if they are executable files
   using is_executable() function */
$files_arr = array(
  "$file_path/is_executable_variation1/bar.tmp",

  /* Testing a file with trailing slash */
  "$file_path/is_executable_variation1/bar.tmp/",

  /* Testing file with double slashes */
  "$file_path/is_executable_variation1//bar.tmp",
  "$file_path/is_executable_variation1/*.tmp",
  "$file_path/is_executable_variation1/b*.tmp", 

  /* Testing Binary safe */
  "$file_path/is_executable_variation1".chr(0)."bar.temp",
  "$file_path".chr(0)."is_executable_variation1/bar.temp",
  "$file_path/is_executable_variation1x000/",
  
  /* Testing directories */
  ".",  // current directory, exp: bool(true)
  "$file_path/is_executable_variation1"  // temp directory, exp: bool(true)
);
$counter = 1;
/* loop through to test each element in the above array 
   is an executable file */
foreach($files_arr as $file) {
  echo "-- Iteration $counter --\n";
  var_dump( is_executable($file) );
  $counter++;
  clearstatcache();
}

echo "Done\n";
?>
<?php
unlink(dirname(__FILE__)."/is_executable_variation1/bar.tmp");
rmdir(dirname(__FILE__)."/is_executable_variation1/");
?>