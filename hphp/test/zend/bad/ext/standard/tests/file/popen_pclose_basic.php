<?php
/*
 *  Prototype: resource popen ( string command, string mode )
 *  Description: Opens process file pointer.
 *
 *  Prototype: int pclose ( resource handle );
 *  Description: Closes process file pointer.
 */

$file_path = dirname(__FILE__);
require($file_path."/file.inc");

echo "*** Testing popen() and pclose() with different processes ***\n";

echo "-- Testing popen(): reading from the pipe --\n";
$dirpath = $file_path."/popen_basic";
mkdir($dirpath);
touch($dirpath."/popen_basic.tmp");
define('CMD', "ls $dirpath");
$file_handle = popen(CMD, 'r');
fpassthru($file_handle);
pclose($file_handle);

echo "-- Testing popen(): reading from a file using 'cat' command --\n";
create_files($dirpath, 1, "text_with_new_line", 0755, 100, "w", "popen_basic", 1, "bytes");
$filename = $dirpath."/popen_basic1.tmp";
$command = "cat $filename";
$file_handle = popen($command, "r");
$return_value =  fpassthru($file_handle);
echo "\n";
var_dump($return_value);
pclose($file_handle);
delete_files($dirpath, 1);

echo "*** Testing popen(): writing to the pipe ***\n";
$arr = array("ggg", "ddd", "aaa", "sss");
$file_handle = popen("sort", "w");
$counter = 0;
$newline = "\n";
foreach($arr as $str) {
  fwrite($file_handle, (binary)$str);
  fwrite($file_handle, (binary)$newline);
}
pclose($file_handle);


echo "*** Testing for return type of popen() and pclose() functions ***\n";
$string = "Test String";
$return_value_popen = popen("echo $string", "r");
var_dump( is_resource($return_value_popen) );
fpassthru($return_value_popen);
$return_value_pclose = pclose($return_value_popen);
var_dump( is_int($return_value_pclose) );

echo "\n--- Done ---";
?>
<?php
$file_path = dirname(__FILE__);
$dirpath = $file_path."/popen_basic";
unlink($dirpath."/popen_basic.tmp");
unlink($dirpath."/popen_basic1.tmp");
rmdir($dirpath);
?>
