<?php
/* Prototype: bool rename ( string $oldname, string $newname [, resource $context] );
   Description: Renames a file or directory
*/

require dirname(__FILE__).'/file.inc';

/* create directory */
$file_path = dirname(__FILE__);
mkdir("$file_path/rename_variation");

/* rename files across directories */
echo "*** Testing rename() : rename files across directories ***\n";
$src_filenames = array(
  "$file_path/rename_variation/rename_variation-win32.phpt.tmp",

  /* Testing a file trailing slash */
  "$file_path/rename_variation/rename_variation-win32.phpt.tmp/",

  /* Testing file with double slashes */
  "$file_path/rename_variation//rename_variation-win32.phpt.tmp",
  "$file_path//rename_variation//rename_variation-win32.phpt.tmp",
);

$counter = 1;

/* loop through each $file and rename it to rename_variation-win32.phpt2.tmp */
foreach($src_filenames as $src_filename) {
  echo "-- Iteration $counter --\n";
  $fp = fopen("$file_path/rename_variation/rename_variation-win32.phpt.tmp", "w");
  fclose($fp);
  $dest_filename = "$file_path/rename_variation-win32.phpt2.tmp";
  var_dump( rename($src_filename, $dest_filename) );

  // ensure that file got renamed to new name 
  var_dump( file_exists($src_filename) );  // expecting false
  var_dump( file_exists($dest_filename) );  // expecting true
  $counter++;
 
  // unlink the file  
  unlink($dest_filename);
}

rmdir("$file_path/rename_variation"); 

echo "Done\n";
?>
<?php
$file_path = dirname(__FILE__);
unlink($file_path."/rename_variation-win32.phpt_link.tmp");
unlink($file_path."/rename_variation-win32.phpt.tmp");
rmdir($file_path."/rename_variation-win32.phpt_dir");
?>