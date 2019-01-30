<?php


$test_dir = getenv('HPHP_TEST_TMPDIR') ?? dirname(__FILE__);
$thisTestDir = $test_dir . '/' .basename(__FILE__, ".php") . ".directory";
mkdir($thisTestDir);
$oldDirPath = getcwd();
chdir($thisTestDir);

$filename = basename(__FILE__, ".php") . ".tmp"; 
$scriptLocFile = dirname(__FILE__)."/".$filename;

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


function runtest() {
   global $scriptLocFile, $filename;
   file_put_contents($filename, (binary) "File written in working directory", FILE_USE_INCLUDE_PATH);
   if(file_exists($scriptLocFile)) {
      echo "Fail - this is PHP52 behaviour\n";
      unlink($scriptLocFile);
   }else {
      $line = file_get_contents($filename); 
      echo "$line\n";
      unlink($filename);     
   }
}
?>
===DONE===
