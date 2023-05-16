<?hh
/* Prototype  : mixed opendir(string $path[, resource $context])
 * Description: Open a directory and return a dir_handle 
 * Source code: ext/standard/dir.c
 */

/*
 * Pass paths containing wildcards to test if opendir() recognises them
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing opendir() : usage variations ***\n";
// create the temporary directories
$file_path = sys_get_temp_dir();
$dir_path = $file_path . "/opendir_variation6";
$sub_dir_path = $dir_path . "/sub_dir1";

mkdir($dir_path);
mkdir($sub_dir_path);

// with different wildcard characters

echo "\n-- Wildcard = '*' --\n"; 
var_dump( opendir($file_path . "/opendir_var*") );
var_dump( opendir($file_path . "/*") );

echo "\n-- Wildcard = '?' --\n";
var_dump( opendir($dir_path . "/sub_dir?") );
var_dump( opendir($dir_path . "/sub?dir1") );

echo "===DONE===\n";

rmdir($sub_dir_path);
rmdir($dir_path);
}
