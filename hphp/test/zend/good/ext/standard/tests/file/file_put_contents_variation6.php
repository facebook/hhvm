<?hh
/* Prototype  : int file_put_contents(string file, mixed data [, int flags [, resource context]])
 * Description: Write/Create a file with contents data and return the number of bytes written
 * Source code: ext/standard/file.c
 * Alias to functions:
 */

function runtest() :mixed{


   //correct php53 behaviour is to ignore the FILE_USE_INCLUDE_PATH unless the file already exists
   // in the include path. In this case it doesn't so the file should be written in the current dir.

   file_put_contents(ZendGoodExtStandardTestsFileFilePutContentsVariation6::$filename, (string)"File in include path", FILE_USE_INCLUDE_PATH);
   file_put_contents(ZendGoodExtStandardTestsFileFilePutContentsVariation6::$filename, (string)". This was appended", FILE_USE_INCLUDE_PATH | FILE_APPEND);
   $line = file_get_contents(ZendGoodExtStandardTestsFileFilePutContentsVariation6::$filename);
   echo "$line\n";
   unlink(ZendGoodExtStandardTestsFileFilePutContentsVariation6::$filename);
}

abstract final class ZendGoodExtStandardTestsFileFilePutContentsVariation6 {
  public static $filename;
}
<<__EntryPoint>> function main(): void {
require_once('fopen_include_path.inc');

echo "*** Testing file_put_contents() : variation ***\n";

$thisTestDir = sys_get_temp_dir().'/'.'dir';
mkdir($thisTestDir);
$oldDirPath = getcwd();
chdir($thisTestDir);

ZendGoodExtStandardTestsFileFilePutContentsVariation6::$filename = basename(__FILE__, ".php") . ".tmp";

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
