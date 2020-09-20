<?hh
/*
   Prototype: int fileatime ( string $filename );
   Description: Returns the time the file was last accessed, or FALSE
     in case of an error. The time is returned as a Unix timestamp.

   Prototype: int filemtime ( string $filename );
   Description: Returns the time the file was last modified, or FALSE
     in case of an error.

   Prototype: int filectime ( string $filename );
   Description: Returns the time the file was last changed, or FALSE
     in case of an error. The time is returned as a Unix timestamp.

   Prototype: bool touch ( string $filename [, int $time [, int $atime]] );
   Description: Attempts to set the access and modification times of the file
     named in the filename parameter to the value given in time.
*/

/*
   Prototype: void stat_fn(string $filename);
   Description: Prints access, modification and change times of a file
*/
function stat_fn( $filename ) {
  echo "-- File access time is => ";
  print( @date( 'Y:M:D:H:i:s', fileatime($filename) ) )."\n";
  clearstatcache();
  echo "-- File modification time is => ";
  print( @date( 'Y:M:D:H:i:s', filemtime($filename) ) )."\n";
  clearstatcache();
  echo "-- inode change time is => ";
  print( @date( 'Y:M:D:H:i:s', filectime($filename) ) )."\n";
  clearstatcache();

}
<<__EntryPoint>> function main(): void {
echo "*** Testing fileattime(), filemtime(), filectime() & touch() : usage variations ***\n";
// create files
$file_handle = fopen(__SystemLib\hphp_test_tmppath('005_variation1.tmp'), 'w');
fclose($file_handle);
stat_fn(__SystemLib\hphp_test_tmppath('005_variation1.tmp'));
sleep(2);

$file_handle = fopen(__SystemLib\hphp_test_tmppath('005_variation2.tmp'), 'w');
fclose($file_handle);
stat_fn(__SystemLib\hphp_test_tmppath('005_variation2.tmp'));
sleep(2);

$file_handle = fopen(__SystemLib\hphp_test_tmppath('005_variation3.tmp'), 'w');
fclose($file_handle);
stat_fn(__SystemLib\hphp_test_tmppath('005_variation3.tmp'));

// delete files
unlink(__SystemLib\hphp_test_tmppath('005_variation1.tmp'));
unlink(__SystemLib\hphp_test_tmppath('005_variation2.tmp'));
unlink(__SystemLib\hphp_test_tmppath('005_variation3.tmp'));

echo "\n-- Checking different times, just after creating the file --\n";
$file_name = __SystemLib\hphp_test_tmppath('005_variation1.tmp');
$file_write_handle = fopen($file_name, "w");
fclose($file_write_handle);
stat_fn($file_name);
sleep(2);

/* filectime + 2 */
echo "\n-- Checking different times, after changing the file permission --\n";
chmod($file_name, 0777);
stat_fn($file_name);
sleep(2);

/* filemtime + 2 & filectime + 2 */
echo "\n-- Checking different times, after writing into the file --\n";
$file_write_handle = fopen($file_name, "w");
fwrite($file_write_handle, "Hello, world");
fclose($file_write_handle);
stat_fn($file_name);
sleep(2);

/* fileatime + 2 */
echo "\n-- Checking different times, after reading from the file --\n";
$file_read_handle = fopen($file_name ,"r");
fread($file_read_handle, 10);
fclose( $file_read_handle);
stat_fn($file_name);
sleep(2);

/* No change */
echo "\n-- Checking different times, after creating a softlink to the file --\n";
symlink($file_name, __SystemLib\hphp_test_tmppath('005_variation_softlink.tmp'));
stat_fn($file_name);
sleep(2);

/* filectime + 2 */
echo "\n-- Checking different times, after creating a hardlink to the file --\n";
link($file_name, __SystemLib\hphp_test_tmppath('005_variation_hardlink.tmp'));
stat_fn($file_name);
sleep(2);

/* No change */
echo "\n-- Checking different times, after making a copy of the file --\n";
$file_copy = __SystemLib\hphp_test_tmppath('005_variation_copy.tmp');
copy($file_name, $file_copy);
stat_fn($file_name);
sleep(2);

/* fileatime + 2 */
echo "\n-- Checking different times, after performing is_file() operation on the file --\n";
is_file($file_name);
stat_fn($file_name);
sleep(2);


echo "\n*** Testing touch() function with different time values ***\n";
$file_name2 = __SystemLib\hphp_test_tmppath('005_variation_touch.tmp');
$file_handle = fopen($file_name2, "w");
fclose($file_handle);
sleep(2);

/* Time is not mentioned */
var_dump( touch($file_name2) ); //set to current system time
stat_fn($file_name2);
sleep(2);

/* set to access(creation time of the file) time */
var_dump( touch($file_name2, (int)@date((string)fileatime($file_name2))) );
stat_fn($file_name2);
sleep(2);

/* set to access time of $file_name2 */
var_dump(touch(__SystemLib\hphp_test_tmppath('005_variation_touch_fly.tmp'), (int)@date((string)fileatime($file_name2)), time()));
stat_fn($file_name2);
sleep(2);

/* set to default value, with Invalid timestamps */
var_dump( touch($file_name2, 10) );
stat_fn($file_name2);
var_dump( touch($file_name2, 10, 20) );
stat_fn($file_name2);

/* touch() after renaming the file */
rename($file_name2, __SystemLib\hphp_test_tmppath('005_variation_touch_new.tmp'));
stat_fn(__SystemLib\hphp_test_tmppath('005_variation_touch_new.tmp'));

echo "Done\n";

unlink(__SystemLib\hphp_test_tmppath('005_variation_softlink.tmp'));
unlink(__SystemLib\hphp_test_tmppath('005_variation_hardlink.tmp'));
unlink(__SystemLib\hphp_test_tmppath('005_variation1.tmp'));
unlink(__SystemLib\hphp_test_tmppath('005_variation_copy.tmp'));
unlink(__SystemLib\hphp_test_tmppath('005_variation_touch_fly.tmp'));
unlink(__SystemLib\hphp_test_tmppath('005_variation_touch_new.tmp'));
}
