<?php
/* Prototype: bool copy ( string $source, string $dest );
   Description: Makes a copy of the file source to dest.
     Returns TRUE on success or FALSE on failure.
*/

/* Test copy() function: Trying to create copy of source file 
     into different destination dir paths given in various notations */

echo "*** Test copy() function: copying file across directories ***\n";
$base_dir = dirname(__FILE__)."/copy_variation6";
mkdir($base_dir);

$sub_dir = $base_dir."/copy_variation6_sub";
mkdir($sub_dir);

$dirname_with_blank = $sub_dir."/copy variation6";
mkdir($dirname_with_blank);

$src_file_name = dirname(__FILE__)."/copy_variation6.tmp";
fclose( fopen($src_file_name, "w") );

echo "Size of source file => ";
var_dump( filesize($src_file_name) );
clearstatcache();

$dests = array(
  $base_dir."/copy_copy_variation6.tmp",
  $base_dir."/copy_variation6_sub/copy_copy_variation6.tmp",
  "$sub_dir/copy_copy_variation6.tmp",
  "$sub_dir/../copy_copy_variation6.tmp",
  "$sub_dir/../copy_variation6_sub/copy_copy_variation6.tmp",
  "$sub_dir/..///../copy_copy_variation6.tmp",
  "$sub_dir/..///../*",
  "$dirname_with_blank/copy_copy_variation6.tmp"
);

echo "\n-- Now applying copy() on source file to create copies --";
$count = 1;
foreach($dests as $dest) {
  echo "\n-- Iteration $count --\n";

  echo "Copy operation => ";
  var_dump( copy($src_file_name, $dest) );

  echo "Existence of destination file => ";
  var_dump( file_exists($dest) );

  echo "Destination file name is => ";
  print($dest);
  echo "\n";

  echo "Size of source file => ";
  var_dump( filesize($src_file_name) );
  clearstatcache();

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
?>
