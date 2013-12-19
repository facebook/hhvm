<?php

/* test rename() by trying to rename an existing file/dir/link to the same name
  and one another */
// create a dir 
$file_path = dirname(__FILE__);
$dirname = "$file_path/rename_variation5.phpt_dir"; 
mkdir($dirname);
//create a file
$filename = "$file_path/rename_variation5.phpt.tmp"; 
$fp = fopen($filename, "w");
fclose($fp);
// create a link
$linkname = "$file_path/rename_variation5.phpt_link.tmp";
symlink($filename, $linkname);

echo "\n-- Renaming link to same link name --\n";
var_dump( rename($linkname, $linkname) );

echo "\n-- Renaming file to same file name --\n";
var_dump( rename($filename, $filename) );

echo "\n-- Renaming directory to same directory name --\n";
var_dump( rename($dirname, $dirname) );

echo "\n-- Renaming existing link to existing directory name --\n";
var_dump( rename($linkname, $dirname) );
echo "\n-- Renaming existing link to existing file name --\n";
var_dump( rename($linkname, $filename) );

echo "\n-- Renaming existing file to existing directory name --\n";
var_dump( rename($filename, $dirname) );
echo "\n-- Renaming existing file to existing link name --\n";
var_dump( rename($filename, $linkname) );

echo "\n-- Renaming existing directory to existing file name --\n";
$fp = fopen($filename, "w");
fclose($fp);
var_dump( rename($dirname, $filename) );
echo "\n-- Renaming existing directory to existing link name --\n";
var_dump( rename($dirname, $linkname) );

echo "Done\n";
?>
<?php
$file_path = dirname(__FILE__);
unlink($file_path."/rename_variation5.phpt_link.tmp");
unlink($file_path."/rename_variation5.phpt.tmp");
rmdir($file_path."/rename_variation5.phpt_dir");
?>