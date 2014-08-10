<?php
/* Prototype  : resource fopen(string filename, string mode [, bool use_include_path [, resource context]])
 * Description: Open a file or a URL and return a file pointer 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */



//create the include directory structure
$thisTestDir =  basename(__FILE__, ".php") . ".dir";
mkdir($thisTestDir);
chdir($thisTestDir);

$workingDir = "workdir";
$filename = basename(__FILE__, ".php") . ".tmp";
$scriptDir = dirname(__FILE__);
$baseDir = getcwd();
$secondFile = $baseDir."/dir2/".$filename;
$firstFile = "../dir1/".$filename;
$scriptFile = $scriptDir.'/'.$filename;

$newdirs = array("dir1", "dir2", "dir3");
$pathSep = ":";
$newIncludePath = "";
if(substr(PHP_OS, 0, 3) == 'WIN' ) {
   $pathSep = ";";
}
foreach($newdirs as $newdir) {
   mkdir($newdir);
   $newIncludePath .= $baseDir.'/'.$newdir.$pathSep;
}
mkdir($workingDir);
chdir($workingDir);

//define the files to go into these directories, create one in dir2
echo "\n--- testing include path ---\n";
set_include_path($newIncludePath);   
$modes = array("r", "r+", "rt");
foreach($modes as $mode) {
    test_fopen($mode);
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


function test_fopen($mode) {
   global $scriptFile, $secondFile, $firstFile, $filename;
   
   // create a file in the middle directory
   $h = fopen($secondFile, "w");
   fwrite($h, (binary) "in dir2");
   fclose($h);

   echo "\n** testing with mode=$mode **\n";
   // should read dir2 file
   $h = fopen($filename, $mode, true);
   fpassthru($h);
   fclose($h);
   echo "\n";

   //create a file in dir1
   $h = fopen($firstFile, "w");
   fwrite($h, (binary) "in dir1");
   fclose($h);
   
   //should now read dir1 file
   $h = fopen($filename, $mode, true);
   fpassthru($h);
   fclose($h);
   echo "\n";
   
   // create a file in working directory
   $h = fopen($filename, "w");
   fwrite($h, (binary) "in working dir");
   fclose($h);
   
   //should still read dir1 file
   $h = fopen($filename, $mode, true);
   fpassthru($h);
   fclose($h);
   echo "\n";
   
   unlink($firstFile);
   unlink($secondFile);
   
   //should read the file in working dir
   $h = fopen($filename, $mode, true);
   fpassthru($h);
   fclose($h);
   echo "\n";
   
   // create a file in the script directory
   $h = fopen($scriptFile, "w");
   fwrite($h, (binary) "in script dir");
   fclose($h);
   
   //should read the file in script dir
   $h = fopen($filename, $mode, true);
   fpassthru($h);
   fclose($h);
   echo "\n";
     
   //cleanup
   unlink($filename);
   unlink($scriptFile);

}

?>
===DONE===
