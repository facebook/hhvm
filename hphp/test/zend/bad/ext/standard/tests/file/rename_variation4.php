<?php

$file_path = dirname(__FILE__);

require dirname(__FILE__).'/file.inc';

/* Renaming a file, link and directory to numeric name */
echo "\n*** Testing rename() by renaming a file, link and directory to numeric name ***\n";
$fp = fopen($file_path."/rename_variation4.phpt.tmp", "w");
fclose($fp);
// renaming existing file to numeric name
var_dump( rename($file_path."/rename_variation4.phpt.tmp", $file_path."/12345") );
// ensure that rename worked fine
var_dump( file_exists($file_path."/rename_variation4.phpt.tmp" ) );  // expecting false
var_dump( file_exists($file_path."/12345" ) );  // expecting true
// remove the file
unlink($file_path."/12345");

mkdir($file_path."/rename_variation4.phpt_dir");

// renaming a directory to numeric name
var_dump( rename($file_path."/rename_variation4.phpt_dir/", $file_path."/12345") );
// ensure that rename worked fine
var_dump( file_exists($file_path."/rename_variation4.phpt_dir" ) );  // expecting false
var_dump( file_exists($file_path."/12345" ) );  // expecting true

echo "Done\n";
?>
<?php
$file_path = dirname(__FILE__);
rmdir($file_path."/12345");
?>