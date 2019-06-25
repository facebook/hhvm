<?hh
/* Prototype  : void closedir([resource $dir_handle])
 * Description: Close directory connection identified by the dir_handle 
 * Source code: ext/standard/dir.c
 * Alias to functions: close
 */

/*
 * Pass incorrect number of arguments to closedir() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing closedir() : error conditions ***\n";


//Test closedir with one more than the expected number of arguments
echo "\n-- Testing closedir() function with more than expected no. of arguments --\n";

$dir_path = dirname(__FILE__) . '\closedir_error';
mkdir($dir_path);
$dir_handle = opendir($dir_path);

$extra_arg = 10;
try { var_dump( closedir($dir_handle, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//successfully close the directory handle so can delete in CLEAN section
closedir($dir_handle);
echo "===DONE===\n";
error_reporting(0);
$base_dir = dirname(__FILE__);
$dir_path = $base_dir . '\closedir_error';
rmdir($dir_path);
}
