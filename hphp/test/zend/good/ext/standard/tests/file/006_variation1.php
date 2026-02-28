<?hh
/* 
  Prototype: int fileperms ( string $filename );
  Description: Returns the permissions on the file, or FALSE in case of an error

  Prototype: bool chmod ( string $filename, int $mode );
  Description: Attempts to change the mode of the file specified by 
               filename to that given in mode
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing fileperms() & chmod() : usage variations ***\n";

$file_name = sys_get_temp_dir().'/'.'006_variation1.tmp';
$file_handle = fopen($file_name, "w");
fclose($file_handle);
$dir_name = sys_get_temp_dir().'/'.'006_variation1';
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

chmod($file_name, 0777);
chmod($dir_name, 0777);
unlink($file_name);
rmdir($dir_name);
}
