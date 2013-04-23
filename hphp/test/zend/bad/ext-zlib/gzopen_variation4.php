<?php
/* Prototype  : resource gzopen(string filename, string mode [, int use_include_path])
 * Description: Open a .gz-file and return a .gz-file pointer 
 * Source code: ext/zlib/zlib.c
 * Alias to functions: 
 */

echo "*** Testing gzopen() : usage variation ***\n";

require_once('reading_include_path.inc');

//define the files to go into these directories, create one in dir2
echo "\n--- testing include path ---\n";
set_include_path($newIncludePath);   
$modes = array("r", "r+", "rt");
foreach($modes as $mode) {
    test_gzopen($mode);
}
restore_include_path();

// remove the directory structure
chdir($baseDir);
rmdir($workingDir);
foreach($newdirs as $newdir) {
   rmdir($newdir);
}

chdir("..");
rmdir($thisTestDir);

function test_gzopen($mode) {
   global $scriptFile, $secondFile, $firstFile, $filename;
   
   // create a file in the middle directory
   $h = gzopen($secondFile, "w");
   gzwrite($h, "This is a file in dir2");
   gzclose($h);

   echo "\n** testing with mode=$mode **\n";
   // should read dir2 file
   $h = gzopen($filename, $mode, true);
   gzpassthru($h);
   gzclose($h);
   echo "\n";

   //create a file in dir1
   $h = gzopen($firstFile, "w");
   gzwrite($h, "This is a file in dir1");
   gzclose($h);
   
   //should now read dir1 file
   $h = gzopen($filename, $mode, true);
   gzpassthru($h);
   gzclose($h);
   echo "\n";
   
   // create a file in working directory
   $h = gzopen($filename, "w");
   gzwrite($h, "This is a file in working dir");
   gzclose($h);
   
   //should still read dir1 file
   $h = gzopen($filename, $mode, true);
   gzpassthru($h);
   gzclose($h);
   echo "\n";
   
   unlink($firstFile);
   unlink($secondFile);
   
   //should read the file in working dir
   $h = gzopen($filename, $mode, true);
   gzpassthru($h);
   gzclose($h);
   echo "\n";
   
   // create a file in the script directory
   $h = gzopen($scriptFile, "w");
   gzwrite($h, "This is a file in script dir");
   gzclose($h);
   
   //should read the file in script dir
   $h = gzopen($filename, $mode, true);
   gzpassthru($h);
   gzclose($h);
   echo "\n";
     
   //cleanup
   unlink($filename);
   unlink($scriptFile);

}

?>
===DONE===