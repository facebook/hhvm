<?hh
/* Prototype  : string file_get_contents(string filename [, bool use_include_path [, resource context [, long offset [, long maxlen]]]])
 * Description: Read the entire file into a string
 * Source code: ext/standard/file.c
 * Alias to functions:
 */

function runtest() :mixed{

   $h = fopen(ZendGoodExtStandardTestsFileFileGetContentsVariation1::$secondFile, "w");
   fwrite($h, "File in include path");
   fclose($h);
   $line = file_get_contents(ZendGoodExtStandardTestsFileFileGetContentsVariation1::$filename, true);
   echo "$line\n";
   unlink(ZendGoodExtStandardTestsFileFileGetContentsVariation1::$secondFile);
}

abstract final class ZendGoodExtStandardTestsFileFileGetContentsVariation1 {
  public static $secondFile;
  public static $filename = "file_get_contents_variation1.txt";
}
<<__EntryPoint>> function main(): void {
require_once('fopen_include_path.inc');

echo "*** Testing file_get_contents() : variation ***\n";

// this doesn't create the include dirs in this directory
// we change to this to ensure we are not part of the
// include paths.
$thisTestDir = sys_get_temp_dir().'/'.'fileGetContentsVar1.dir';
mkdir($thisTestDir);
chdir($thisTestDir);


ZendGoodExtStandardTestsFileFileGetContentsVariation1::$secondFile = ZendGoodExtStandardTestsFileFopenIncludePathInc::dir2()."/".ZendGoodExtStandardTestsFileFileGetContentsVariation1::$filename;

$newpath = create_include_path();
set_include_path($newpath);
runtest();
teardown_include_path();
restore_include_path();

rmdir($thisTestDir);

echo "===DONE===\n";
}
