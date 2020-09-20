<?hh
/* Prototype  : bool chdir(string $directory)
 * Description: Change the current directory 
 * Source code: ext/standard/dir.c
 */

/*
 * Pass incorrect number of arguments to chdir() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing chdir() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing chdir() function with Zero arguments --\n";
try { var_dump( chdir() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test chdir with one more than the expected number of arguments
echo "\n-- Testing chdir() function with more than expected no. of arguments --\n";
$directory = __FILE__;
$extra_arg = 10;
try { var_dump( chdir($directory, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
}
