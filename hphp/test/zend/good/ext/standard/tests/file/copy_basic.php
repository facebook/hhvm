<?hh
/* Prototype: bool copy ( string $source, string $dest );
 * Description: Makes a copy of the file source to dest.
 *              Returns TRUE on success or FALSE on failure.
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing copy() function: to copy file from source to destination --\n"; 

var_dump( file_exists(__FILE__) );

/* copying the file */
$file_name1 = __SystemLib\hphp_test_tmppath('copy_basic1.tmp');
$file_name2 = __SystemLib\hphp_test_tmppath('copy_basic2.tmp');
var_dump( copy(__FILE__, $file_name1) );
var_dump( copy($file_name1, $file_name2) );

echo "-- Checking whether the copy of file exists --\n";
var_dump( file_exists($file_name1) );
var_dump( file_exists($file_name2) );

echo "-- Checking filepermissions of file and its copies --\n";
printf( "%o", fileperms(__FILE__) );
echo "\n";
printf( "%o", fileperms($file_name1) );
echo "\n";
printf( "%o", fileperms($file_name2) );
echo "\n";

echo "*** Done ***\n";

unlink($file_name1);
unlink($file_name2);
}
