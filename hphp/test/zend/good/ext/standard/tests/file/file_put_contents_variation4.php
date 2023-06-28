<?hh
/* Prototype  : int file_put_contents(string file, mixed data [, int flags [, resource context]])
 * Description: Write/Create a file with contents data and return the number of bytes written
 * Source code: ext/standard/file.c
 * Alias to functions:
 */

function runtest() :mixed{

   //correct php53 behaviour is to ingnore the FILE_USE_INCLUDE_PATH unless the file alread exists
   // in the include path. In this case it doesn't so the file should be written in the current dir.
   file_put_contents(ZendGoodExtStandardTestsFileFilePutContentsVariation4::$filename, (string)"File in include path", FILE_USE_INCLUDE_PATH);
   $line = file_get_contents(ZendGoodExtStandardTestsFileFilePutContentsVariation4::$filename);
   echo "$line\n";
   unlink(ZendGoodExtStandardTestsFileFilePutContentsVariation4::$filename);
}

abstract final class ZendGoodExtStandardTestsFileFilePutContentsVariation4 {
  public static $filename;
}
<<__EntryPoint>> function main(): void {
require_once('fopen_include_path.inc');
$thisTestDir = sys_get_temp_dir().'/'.'dir';

mkdir($thisTestDir);
$oldDirPath = getcwd();
chdir($thisTestDir);

ZendGoodExtStandardTestsFileFilePutContentsVariation4::$filename = basename(__FILE__, ".php") . ".tmp";

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
