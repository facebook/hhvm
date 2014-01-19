<?php
/* 
  Prototype: int fileperms ( string $filename );
  Description: Returns the permissions on the file, or FALSE in case of an error

  Prototype: bool chmod ( string $filename, int $mode );
  Description: Attempts to change the mode of the file specified by 
               filename to that given in mode
*/

/* Testing with miscellaneous Permission */

echo "*** Testing fileperms() & chmod() : usage variations ***\n";

$file_name = dirname(__FILE__)."/006_variation2.tmp";
$file_handle = fopen($file_name, "w");
fclose($file_handle);
$dir_name = dirname(__FILE__)."/006_variation2";
mkdir($dir_name);

echo "\n*** Testing fileperms(), chmod() with miscellaneous permissions ***\n";
$perms_array = array(
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

  /* strings notation of permission,  wont work properly */
  "r+w",
  "r+w+x",
  "u+rwx",
  "u+rwx, g+rw, o+wx"
);

$count = 1;
foreach($perms_array as $permission) {
  echo "-- Iteration $count --\n";
  var_dump( chmod($file_name, $permission) );
  printf("%o", fileperms($file_name) );
  echo "\n";
  clearstatcache();
 
  var_dump( chmod($dir_name, $permission) );
  printf("%o", fileperms($dir_name) );
  echo "\n";
  clearstatcache();
  $count++;
}
echo "*** Done ***\n";
?>
<?php
chmod(dirname(__FILE__)."/006_variation2.tmp", 0777);
chmod(dirname(__FILE__)."/006_variation2", 0777);
unlink(dirname(__FILE__)."/006_variation2.tmp");
rmdir(dirname(__FILE__)."/006_variation2");
?>