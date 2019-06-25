<?hh
/* Prototype  : void rewinddir([resource $dir_handle])
 * Description: Rewind dir_handle back to the start 
 * Source code: ext/standard/dir.c
 * Alias to functions: rewind
 */

/*
 * Pass incorrect number of arguments to rewinddir() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing rewinddir() : error conditions ***\n";


//Test rewinddir with one more than the expected number of arguments
echo "\n-- Testing rewinddir() function with more than expected no. of arguments --\n";

$dir_path = dirname(__FILE__) . "/rewinddir_error";
mkdir($dir_path);
$dir_handle = opendir($dir_path);
$extra_arg = 10;

try { var_dump( rewinddir($dir_handle, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
closedir($dir_handle);
echo "===DONE===\n";
error_reporting(0);
$dir_path = dirname(__FILE__) . "/rewinddir_error";
rmdir($dir_path);
}
