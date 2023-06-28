<?hh
/* Prototype  : resource fopen(string filename, string mode [, bool use_include_path [, resource context]])
 * Description: Open a file or a URL and return a file pointer
 * Source code: ext/standard/file.c
 * Alias to functions:
 */

function runtest() :mixed{
    $tmpfile =  basename(__FILE__, ".php") . ".tmp";
    $h = fopen($tmpfile, "w", true);
    fwrite($h, (string)"This is the test file");
    fclose($h);


    $h = @fopen($tmpfile, "r");
    if ($h === false) {
       echo "Not created in working dir\n";
    }
    else {
       echo "created in working dir\n";
       fclose($h);
       unlink($tmpfile);
    }

    $h = @fopen('dir1/'.$tmpfile, "r");
    if ($h === false) {
       echo "Not created in dir1\n";
    }
    else {
       echo "created in dir1\n";
       fclose($h);
       unlink('dir1/'.$tmpfile);
    }
}
<<__EntryPoint>> function main(): void {
require_once('fopen_include_path.inc');

$thisTestDir =  sys_get_temp_dir().'/'.'fopen_variation9.dir';
mkdir($thisTestDir);
chdir($thisTestDir);

$newpath = relative_include_path();
set_include_path($newpath);
runtest();

$newpath = generate_next_rel_path();
set_include_path($newpath);
runtest();

teardown_relative_path();
restore_include_path();

rmdir($thisTestDir);

echo "===DONE===\n";
}
