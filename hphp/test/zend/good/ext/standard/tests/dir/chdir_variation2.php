<?hh
/* Prototype  : bool chdir(string $directory)
 * Description: Change the current directory 
 * Source code: ext/standard/dir.c
 */

/*
 * Test chdir() with variations of relative paths
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing chdir() : usage variations ***\n";

$base_dir_path = sys_get_temp_dir();

$level2_one_dir_name = "level2_one";
$level2_one_dir_path = "$base_dir_path/$level2_one_dir_name";

$level2_two_dir_name = "level2_two";
$level2_two_dir_path = "$base_dir_path/$level2_one_dir_name/$level2_two_dir_name";

// create directories
mkdir($level2_one_dir_path);
mkdir($level2_two_dir_path);

echo "\n-- \$directory = './level2_one': --\n";
var_dump(chdir($base_dir_path));
var_dump(chdir("./$level2_one_dir_name"));
var_dump(getcwd());

echo "\n-- \$directory = 'level2_one/level2_two': --\n";
var_dump(chdir($base_dir_path));
var_dump(chdir("$level2_one_dir_name/$level2_two_dir_name"));
var_dump(getcwd());

echo "\n-- \$directory = '..': --\n";
var_dump(chdir('..'));
var_dump(getcwd());

echo "\n-- \$directory = 'level2_two', '.': --\n";
var_dump(chdir($level2_two_dir_path));
var_dump(chdir('.'));
var_dump(getcwd());

echo "\n-- \$directory = '../': --\n";
var_dump(chdir('../'));
var_dump(getcwd());

echo "\n-- \$directory = './': --\n";
var_dump(chdir($level2_two_dir_path));
var_dump(chdir('./'));
var_dump(getcwd());

echo "\n-- \$directory = '../../'level2_one': --\n";
var_dump(chdir($level2_two_dir_path));
var_dump(chdir("../../$level2_one_dir_name"));
var_dump(getcwd());

echo "===DONE===\n";

rmdir($level2_two_dir_path);
rmdir($level2_one_dir_path);
}
