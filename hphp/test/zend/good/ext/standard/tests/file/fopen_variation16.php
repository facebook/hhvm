<?hh
/* Prototype  : resource fopen(string filename, string mode [, bool use_include_path [, resource context]])
 * Description: Open a file or a URL and return a file pointer
 * Source code: ext/standard/file.c
 * Alias to functions:
 */

function runtest() {


    $extraDir = "extraDir16";

    mkdir(ZendGoodExtStandardTestsFileFopenIncludePathInc::dir1().'/'.$extraDir);
    mkdir($extraDir);

    $tmpfile = $extraDir.'/fopen_variation16.tmp';

    $h = fopen($tmpfile, "w+", true);
    fwrite($h, (string)"This is the test file");
    fclose($h);

    $h = @fopen(ZendGoodExtStandardTestsFileFopenIncludePathInc::dir1().'/'.$tmpfile, "r");
    if ($h === false) {
       echo "Not created in dir1\n";
    }
    else {
       echo "created in dir1\n";
       fclose($h);
    }

    $h = fopen($tmpfile, "r", true);
    if ($h === false) {
       echo "could not find file for reading\n";
    }
    else {
       echo "found file - not in dir1\n";
       fclose($h);
    }

    unlink($tmpfile);
    rmdir(ZendGoodExtStandardTestsFileFopenIncludePathInc::dir1().'/'.$extraDir);
    rmdir($extraDir);
}
<<__EntryPoint>> function main(): void {
require_once('fopen_include_path.inc');
$thisTestDir = __SystemLib\hphp_test_tmppath('fopenVariation16.dir');

mkdir($thisTestDir);
$oldDirPath = getcwd();
chdir($thisTestDir);

$newpath = create_include_path();
set_include_path($newpath);
runtest();

$newpath = generate_next_path();
set_include_path($newpath);
runtest();

teardown_include_path();
restore_include_path();
chdir($oldDirPath);
rmdir($thisTestDir);

echo "===DONE===\n";
}
