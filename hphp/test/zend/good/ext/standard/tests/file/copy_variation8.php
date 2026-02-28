<?hh
/* Prototype: bool copy ( string $source, string $dest );
   Description: Makes a copy of the file source to dest.
     Returns TRUE on success or FALSE on failure.
*/

/* Trying to copy the links across dir paths given in various notations
     and dirs having limited access */
<<__EntryPoint>> function main(): void {
echo "*** Testing copy() function: copying links across different directories ***\n";

$base_dir = sys_get_temp_dir().'/'.'copy_variation8';
mkdir($base_dir);
$sub_dir = $base_dir."/copy_variation8_sub";
mkdir($sub_dir);
$dirname_with_blank = $sub_dir."/copy variation6";
mkdir($dirname_with_blank);

$file = sys_get_temp_dir().'/'.'copy_variation8.tmp';
fclose( fopen($file, "w") );

$symlink = sys_get_temp_dir().'/'.'copy_variation8_symlink.tmp';
$hardlink = sys_get_temp_dir().'/'.'copy_variation8_hardlink.tmp';

symlink($file, $symlink);  //creating symlink
link($file, $hardlink);  //creating hardlink

$dests = vec[
  $base_dir."/copy_copy_variation8.tmp",
  $base_dir."/copy_variation8_sub/copy_copy_variation8.tmp",
  "$sub_dir/copy_copy_variation8.tmp",
  "$sub_dir/../copy_copy_variation8.tmp",
  "$sub_dir/../copy_variation8_sub/copy_copy_variation8.tmp",
  "$sub_dir/..///../copy_copy_variation8.tmp",
  "$sub_dir/..///../*",
  "$dirname_with_blank/copy_copy_variation8.tmp"
];

$count = 1;
foreach($dests as $dest) {
  echo "\n-- Iteration $count --\n";
  echo "- With symlink -\n";
  var_dump( copy($symlink, $dest) );
  var_dump( file_exists($dest) );
  var_dump( is_link($dest) ); //expected: bool(false)
  var_dump( is_file($dest) );  //expected: bool(true)
  clearstatcache();
  unlink("$dest");
  echo "- With hardlink -\n";
  var_dump( copy($hardlink, $dest) );
  var_dump( file_exists($dest) );
  var_dump( is_link($dest) );  //expected: bool(false)
  var_dump( is_file($dest) );  //expected: bool(true)
  clearstatcache();
  unlink("$dest");
  $count++;
}

unlink($symlink);
unlink($hardlink);
unlink($file);
rmdir($dirname_with_blank);
rmdir($sub_dir);
rmdir($base_dir);

echo "*** Done ***\n";
}
