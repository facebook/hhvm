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
  var_dump( mkdir("$file_path/mkdir_variation1/", $mode, true) );
  var_dump( rmdir("$file_path/mkdir_variation1/") );
  $counter++;
}

echo "Done\n";
?>