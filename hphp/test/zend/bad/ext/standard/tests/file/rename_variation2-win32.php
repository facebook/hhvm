<?php
/* Prototype: bool rename ( string $oldname, string $newname [, resource $context] );
   Description: Renames a file or directory
*/

require dirname(__FILE__).'/file.inc';

$file_path = dirname(__FILE__);
mkdir("$file_path/rename_variation2-win32.phpt_dir");

/* Renaming a file and directory to numeric name */
echo "\n*** Testing rename() by renaming a file and directory to numeric name ***\n";
$fp = fopen($file_path."/rename_variation2-win32.phpt.tmp", "w");
fclose($fp);

// renaming existing file to numeric name
var_dump( rename($file_path."/rename_variation2-win32.phpt.tmp", $file_path."/12345") );

// ensure that rename worked fine
var_dump( file_exists($file_path."/rename_variation2-win32.phpt.tmp" ) );  // expecting false
var_dump( file_exists($file_path."/12345" ) );  // expecting true

unlink($file_path."/12345");

// renaming a directory to numeric name
var_dump( rename($file_path."/rename_variation2-win32.phpt_dir/", $file_path."/12345") );

// ensure that rename worked fine
var_dump( file_exists($file_path."/rename_variation2-win32.phpt_dir" ) );  // expecting false
var_dump( file_exists($file_path."/12345" ) );  // expecting true

rmdir($file_path."/12345");

echo "Done\n";
?>
<?php
$file_path = dirname(__FILE__);
unlink($file_path."/rename_variation2-win32.phpt_link.tmp");
unlink($file_path."/rename_variation2-win32.phpt.tmp");
rmdir($file_path."/rename_variation2-win32.phpt_dir");
?>