<?php
/*  Prototype: bool mkdir ( string $pathname [, int $mode [, bool $recursive [, resource $context]]] );
    Description: Makes directory
*/

echo "*** Testing mkdir() and rmdir() for different permissions ***\n";

$context = stream_context_create();

$file_path = dirname(__FILE__);
$counter = 1;

for($mode = 0000; $mode <= 0777; $mode++) {
  echo "-- Changing mode of directory to $mode --\n";
  var_dump( mkdir("$file_path/mkdir/", $mode, true) );
  var_dump( rmdir("$file_path/mkdir/") );
  $counter++;
}

echo "\n*** Testing mkdir() and rmdir() by giving stream context as fourth argument ***\n";
var_dump( mkdir("$file_path/mkdir/test/", 0777, true, $context) );
var_dump( rmdir("$file_path/mkdir/test/", $context) );

echo "\n*** Testing rmdir() on a non-empty directory ***\n";
var_dump( mkdir("$file_path/mkdir/test/", 0777, true) );
var_dump( rmdir("$file_path/mkdir/") );

echo "\n*** Testing mkdir() and rmdir() for binary safe functionality ***\n";
var_dump( mkdir("$file_path/tempx000/") );
var_dump( rmdir("$file_path/tempx000/") );

echo "\n*** Testing mkdir() with miscelleneous input ***\n";
/* changing mode of mkdir to prevent creating sub-directory under it */
var_dump( chmod("$file_path/mkdir/", 0000) );
/* creating sub-directory test1 under mkdir, expected: false */
var_dump( mkdir("$file_path/mkdir/test1", 0777, true) );
var_dump( chmod("$file_path/mkdir/", 0777) );  // chmod to enable removing test1 directory

echo "Done\n";
?>
<?php
rmdir(dirname(__FILE__)."/mkdir/test/");
rmdir(dirname(__FILE__)."/mkdir/test1/");
rmdir(dirname(__FILE__)."/mkdir/");
?>