<?hh
/* Prototype: bool rename ( string $oldname, string $newname [, resource $context] );
   Description: Renames a file or directory
*/
<<__EntryPoint>> function main(): void {
echo "\n*** Testing rename() by giving stream context as third argument ***\n";

$context = stream_context_create();

// on directory
$dir_name = sys_get_temp_dir().'/'.'rename_variation9.phpt_dir9';
$new_dir_name = sys_get_temp_dir().'/'.'rename_variation9.phpt_dir9_new';

mkdir($dir_name);

var_dump( rename($dir_name, $new_dir_name, $context) );
var_dump( file_exists($dir_name) );  // expecting false
var_dump( file_exists($new_dir_name) ); // expecting true

//on file
$src_name = sys_get_temp_dir().'/'.'rename_variation9.tmp';
$dest_name = sys_get_temp_dir().'/'.'rename_variation9_new.tmp';

// create the file
$fp = fopen($src_name, "w");
$s1 = stat($src_name);
fclose($fp);

var_dump( rename($src_name, $dest_name, $context) );
var_dump( file_exists($src_name) );  // expecting false
var_dump( file_exists($dest_name) );  // expecting true

echo "Done\n";

unlink(sys_get_temp_dir().'/'.'rename_variation9_new.tmp');
rmdir(sys_get_temp_dir().'/'.'rename_variation9.phpt_dir9_new');
}
