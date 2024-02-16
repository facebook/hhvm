<?hh
/*
  Prototype: int fileperms ( string $filename );
  Description: Returns the permissions on the file, or FALSE in case of an error

  Prototype: bool chmod ( string $filename, int $mode );
  Description: Attempts to change the mode of the file specified by
               filename to that given in mode
*/

/* Testing with miscellaneous Permission */
<<__EntryPoint>> function main(): void {
echo "*** Testing fileperms() & chmod() : usage variations ***\n";

$file_name = sys_get_temp_dir().'/'.'006_variation2.tmp';
$file_handle = fopen($file_name, "w");
try { fclose($file_handle); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
$dir_name = sys_get_temp_dir().'/'.'006_variation2';
mkdir($dir_name);

echo "\n*** Testing fileperms(), chmod() with miscellaneous permissions ***\n";
$perms_array = vec[
  /* testing sticky bit */
  07777,
  00000,
  01000,
  011111,
  /* negative  values as permission */
  -0777, // permissions will be given as 2's complement form of -0777
  -07777, // permissions will be given as 2's complement form of -07777

  /* decimal values as permission  */
  777, // permissions will be given as octal equivalent value of 777
  7777, // permissions will be given as octal equivalent value of 7777
  -7777, // permissions are given as 2's complement of octal equivalent of 7777

  /* hex value as permission */
  0x777, // permissions will be given as ocatal equivalent value of 0x777
  0x7777,
];

$count = 1;
foreach($perms_array as $permission) {
  echo "-- Iteration $count --\n";
  try { var_dump( chmod($file_name, $permission) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  printf("%o", fileperms($file_name) );
  echo "\n";
  clearstatcache();

  try { var_dump( chmod($dir_name, $permission) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  printf("%o", fileperms($dir_name) );
  echo "\n";
  clearstatcache();
  $count++;
}
echo "*** Done ***\n";

unlink($file_name);
rmdir($dir_name);
}
