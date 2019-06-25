<?hh
/* Prototype  : mixed getcwd(void)
 * Description: Gets the current directory 
 * Source code: ext/standard/dir.c
 */

/*
 * Pass incorrect number of arguments to getcwd() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing getcwd() : error conditions ***\n";

// One argument
echo "\n-- Testing getcwd() function with one argument --\n";
$extra_arg = 10;
try { var_dump( getcwd($extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
}
