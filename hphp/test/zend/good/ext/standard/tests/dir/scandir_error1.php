<?hh
/* Prototype  : array scandir(string $dir [, int $sorting_order [, resource $context]])
 * Description: List files & directories inside the specified path 
 * Source code: ext/standard/dir.c
 */

/*
 * Pass incorrect number of arguments to scandir() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing scandir() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing scandir() function with Zero arguments --\n";
try { var_dump( scandir() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test scandir with one more than the expected number of arguments
echo "\n-- Testing scandir() function with more than expected no. of arguments --\n";
$dir = sys_get_temp_dir().'/'.'scandir_error';
mkdir($dir);
$sorting_order = 10;
$context = stream_context_create();
$extra_arg = 10;
try { var_dump( scandir($dir, $sorting_order, $context, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";

rmdir($dir);
}
