<?hh

function runtest() :mixed{

   file_put_contents(ZendGoodExtStandardTestsFileFilePutContentsVariation5::$filename, (string)"File written in working directory", FILE_USE_INCLUDE_PATH);
   if(file_exists(ZendGoodExtStandardTestsFileFilePutContentsVariation5::$scriptLocFile)) {
      echo "Fail - this is PHP52 behaviour\n";
      unlink(ZendGoodExtStandardTestsFileFilePutContentsVariation5::$scriptLocFile);
   }else {
      $line = file_get_contents(ZendGoodExtStandardTestsFileFilePutContentsVariation5::$filename);
      echo "$line\n";
      unlink(ZendGoodExtStandardTestsFileFilePutContentsVariation5::$filename);
   }
}

abstract final class ZendGoodExtStandardTestsFileFilePutContentsVariation5 {
  public static $scriptLocFile;
  public static $filename;
}

<<__EntryPoint>> function main(): void {
$thisTestDir = sys_get_temp_dir().'/'.'directory';
mkdir($thisTestDir);
$oldDirPath = getcwd();
chdir($thisTestDir);

ZendGoodExtStandardTestsFileFilePutContentsVariation5::$filename = basename(__FILE__, ".php") . ".tmp";
ZendGoodExtStandardTestsFileFilePutContentsVariation5::$scriptLocFile = dirname(__FILE__)."/".ZendGoodExtStandardTestsFileFilePutContentsVariation5::$filename;

$newpath = "rubbish";
set_include_path($newpath);
runtest();
$newpath = "";
set_include_path($newpath);
runtest();
set_include_path(null);
runtest();
set_include_path(";;  ; ;c:\\rubbish");
runtest();

chdir($oldDirPath);
rmdir($thisTestDir);

echo "===DONE===\n";
}
