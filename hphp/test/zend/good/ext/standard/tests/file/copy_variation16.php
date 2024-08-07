<?hh
/* Prototype: bool copy ( string $source, string $dest );
   Description: Makes a copy of the file source to dest.
     Returns TRUE on success or FALSE on failure.
*/

/* Test copy() function: Trying to create copy of source file 
     into different destination dir paths given in various notations */
<<__EntryPoint>> function main(): void {
echo "*** Testing copy() function: copying data file across directories ***\n";
$base_dir = sys_get_temp_dir().'/'.'copy_variation16';
mkdir($base_dir);

$sub_dir = $base_dir."/copy_variation16_sub";
mkdir($sub_dir);

$dirname_with_blank = $sub_dir."/copy variation16";
mkdir($dirname_with_blank);

$src_file_name = sys_get_temp_dir().'/'.'copy_variation16.tmp';
$file_handle = fopen($src_file_name, "w");
fwrite($file_handle, str_repeat("Hello world, this is 2007 year ...\n", 100));
fclose($file_handle);

echo "- Size of source file => ";
var_dump( filesize($src_file_name) );
clearstatcache();

$dests = vec[
  $base_dir."/copy_copy_variation16.tmp",
  $base_dir."/copy_variation16_sub/copy_copy_variation16.tmp",
  "$sub_dir/copy_copy_variation16.tmp",
  "$sub_dir/../copy_copy_variation16.tmp",
  "$sub_dir/../copy_variation16_sub/copy_copy_variation16.tmp",
  "$sub_dir/..///../copy_copy_variation16.tmp",
  "$sub_dir/..///../*",
  "$dirname_with_blank/copy_copy_variation16.tmp"
];

echo "\n--- Now applying copy() on source file to create copies ---";
$count = 1;
foreach($dests as $dest) {
  echo "\n-- Iteration $count --\n";

  echo "Size of source file => ";
  var_dump( filesize($src_file_name) );

  echo "Copy operation => ";
  var_dump( copy($src_file_name, $dest) );

  echo "Existence of destination file => ";
  var_dump( file_exists($dest) );

  echo "Destination file name is => ";
  print($dest);
  echo "\n";

  echo "Size of destination file => ";
  var_dump( filesize($dest) );
  clearstatcache();

  unlink("$dest");

  $count++;
}

unlink($src_file_name);
rmdir($dirname_with_blank);
rmdir($sub_dir);
rmdir($base_dir);

echo "*** Done ***\n";
}
