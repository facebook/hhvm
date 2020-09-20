<?hh
/* Prototype  : bool chdir(string $directory)
 * Description: Change the current directory 
 * Source code: ext/standard/dir.c
 */

/*
 * Pass a directory that does not exist as $directory to chdir() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing chdir() : error conditions ***\n";

$directory = __FILE__ . '/idonotexist';

var_dump(chdir($directory));
echo "===DONE===\n";
}
