<?hh
/* Prototype  : mixed getcwd(void)
 * Description: Gets the current directory 
 * Source code: ext/standard/dir.c 
 */

/*
 * Test basic functionality of getcwd()
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing getcwd() : basic functionality ***\n";

//create temporary directory for test, removed in CLEAN section
$directory = sys_get_temp_dir().'/'.'getcwd_basic';
mkdir($directory);

var_dump(getcwd());
chdir($directory);
var_dump(getcwd());
echo "===DONE===\n";

rmdir($directory);
}
