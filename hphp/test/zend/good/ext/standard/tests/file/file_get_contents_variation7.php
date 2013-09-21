<?php
/* Prototype  : string file_get_contents(string filename [, bool use_include_path [, resource context [, long offset [, long maxlen]]]])
 * Description: Read the entire file into a string 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */

echo "*** Testing file_get_contents() : variation ***\n";
$mainDir = "fileGetContentsVar7.dir";
$subDir = "fileGetContentsVar7Sub";
$absMainDir = dirname(__FILE__)."/".$mainDir;
mkdir($absMainDir);
$absSubDir = $absMainDir."/".$subDir;
mkdir($absSubDir);

$old_dir_path = getcwd();
chdir(dirname(__FILE__));

$allDirs = array(
  // absolute paths
  "$absSubDir/",
  "$absSubDir/../".$subDir,
  "$absSubDir//.././".$subDir,
  "$absSubDir/../../".$mainDir."/./".$subDir,
  "$absSubDir/..///".$subDir."//..//../".$subDir,
  "$absSubDir/BADDIR",
  
  // relative paths
  $mainDir."/".$subDir,
  $mainDir."//".$subDir, 
   $mainDir."///".$subDir, 
  "./".$mainDir."/../".$mainDir."/".$subDir,
  "BADDIR",  
  
);

$filename = 'FileGetContentsVar7.tmp';
$absFile = $absSubDir.'/'.$filename;
$h = fopen($absFile,"w");
fwrite($h, "contents read");
fclose($h);

for($i = 0; $i<count($allDirs); $i++) {
  $j = $i+1;
  $dir = $allDirs[$i];
  echo "\n-- Iteration $j --\n";
  var_dump(file_get_contents($dir."/".$filename));
}

chdir($old_dir_path);
unlink($absFile);
rmdir($absSubDir);
rmdir($absMainDir);

echo "\n*** Done ***\n";
?>