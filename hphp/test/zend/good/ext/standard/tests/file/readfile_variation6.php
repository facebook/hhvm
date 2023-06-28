<?hh
/* Prototype  : int readfile(string filename [, bool use_include_path[, resource context]])
 * Description: Output a file or a URL
 * Source code: ext/standard/file.c
 * Alias to functions:
 */

function runtest() :mixed{

   $h = fopen(ZendGoodExtStandardTestsFileReadfileVariation6::$secondFile, "w");
   fwrite($h, "File in include path");
   fclose($h);
   readfile(ZendGoodExtStandardTestsFileReadfileVariation6::$filename, true);
   echo "\n";
   unlink(ZendGoodExtStandardTestsFileReadfileVariation6::$secondFile);
}

abstract final class ZendGoodExtStandardTestsFileReadfileVariation6 {
  public static $secondFile;
  public static $filename = "readfile_variation6.txt";
}
<<__EntryPoint>> function main(): void {
require_once('fopen_include_path.inc');

echo "*** Testing readfile() : variation ***\n";
// this doesn't create the include dirs in this directory
// we change to this to ensure we are not part of the
// include paths.
$thisTestDir = tempnam(sys_get_temp_dir(), 'rfv6');
unlink($thisTestDir);
mkdir($thisTestDir);
chdir($thisTestDir);


ZendGoodExtStandardTestsFileReadfileVariation6::$secondFile = ZendGoodExtStandardTestsFileFopenIncludePathInc::dir2()."/".ZendGoodExtStandardTestsFileReadfileVariation6::$filename;

$newpath = create_include_path();
set_include_path($newpath);
runtest();
teardown_include_path();
restore_include_path();
chdir("..");
rmdir($thisTestDir);


echo "===DONE===\n";
}
