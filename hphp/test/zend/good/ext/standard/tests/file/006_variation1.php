<?php
/* 
  Prototype: int fileperms ( string $filename );
  Description: Returns the permissions on the file, or FALSE in case of an error

  Prototype: bool chmod ( string $filename, int $mode );
  Description: Attempts to change the mode of the file specified by 
               filename to that given in mode
*/

echo "*** Testing fileperms() & chmod() : usage variations ***\n";

$file_name = dirname(__FILE__)."/006_variation1.tmp";
$file_handle = fopen($file_name, "w");
fclose($file_handle);
$dir_name = dirname(__FILE__)."/006_variation1";
mkdir($dir_name);

$count = 1;
echo "-- Testing all permission from octal 0000 to octal 0777 on file and dir --\n";
for($mode = 0000; $mode <= 0777; $mode++) {
  echo "-- Iteration $count --\n";
  var_dump( chmod($file_name, $mode) ); 
  printf("%o", fileperms($file_name) );
  echo "\n";
  clearstatcache();

  var_dump( chmod($dir_name, $mode) );
  printf("%o", fileperms($dir_name) );
  echo "\n";
  clearstatcache();
  $count++;
}

echo "*** Done ***\n";
?>
<?php
chmod(dirname(__FILE__)."/006_variation1.tmp", 0777);
chmod(dirname(__FILE__)."/006_variation1", 0777);
unlink(dirname(__FILE__)."/006_variation1.tmp");
rmdir(dirname(__FILE__)."/006_variation1");
?>