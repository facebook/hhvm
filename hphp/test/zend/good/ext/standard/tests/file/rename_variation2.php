<?hh
<<__EntryPoint>> function main(): void {

$dest_dir = sys_get_temp_dir().'/'.'rename_variation2_dir';
// create the $dest_dir
mkdir($dest_dir);

/* Testing rename() on soft and hard links with different permissions */
echo "\n*** Testing rename() on soft links ***\n";
// create the file
$filename = sys_get_temp_dir().'/'.'rename_variation2.phpt2.tmp';
@unlink($filename);
var_dump(touch($filename));

// create the soft links to the file
$linkname = sys_get_temp_dir().'/'.'rename_variation2_soft_link1.tmp';
var_dump(symlink($filename, $linkname));

//rename the link to a new name in the same dir
$dest_linkname = sys_get_temp_dir().'/'.'rename_variation2_soft_link2.tmp';
var_dump( rename( $linkname, $dest_linkname) );
//ensure that link was renamed 
clearstatcache();
var_dump( file_exists($linkname) );  // expecting false
var_dump( file_exists($dest_linkname) );  // expecting true

// rename a link across dir
var_dump( rename($dest_linkname, $dest_dir."/rename_variation2_soft_link2.tmp"));
//ensure that link got renamed
clearstatcache();
var_dump( file_exists($dest_linkname) );  // expecting false
var_dump( file_exists($dest_dir."/rename_variation2_soft_link2.tmp") ); // expecting true

// delete the link file now 
unlink($dest_dir."/rename_variation2_soft_link2.tmp");

echo "Done\n";

unlink(sys_get_temp_dir().'/'.'rename_variation2.phpt2.tmp');
rmdir(sys_get_temp_dir().'/'.'rename_variation2_dir');
}
