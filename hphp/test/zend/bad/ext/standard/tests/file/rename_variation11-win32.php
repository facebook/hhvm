<?php
/* Prototype  : bool rename(string old_name, string new_name[, resource context])
 * Description: Rename a file 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */

echo "*** Testing rename() with absolute and relative paths ***\n";
$mainDir = "renameVar11";
$subDir = "renameVar11Sub";
$absMainDir = dirname(__FILE__)."\\".$mainDir;
mkdir($absMainDir);
$absSubDir = $absMainDir."\\".$subDir;
mkdir($absSubDir);

$fromFile = "renameMe.tmp";
$toFile = "IwasRenamed.tmp";

$old_dir_path = getcwd();
chdir(dirname(__FILE__));
$unixifiedDir = '/'.substr(str_replace('\\','/',$absSubDir),3);


$allDirs = array(
  // absolute paths
  "$absSubDir\\",
  "$absSubDir\\..\\".$subDir,
  "$absSubDir\\\\..\\.\\".$subDir,
  "$absSubDir\\..\\..\\".$mainDir."\\.\\".$subDir,
  "$absSubDir\\..\\\\\\".$subDir."\\\\..\\\\..\\".$subDir,
  "$absSubDir\\BADDIR",
  
  // relative paths
  $mainDir."\\".$subDir,
  $mainDir."\\\\".$subDir, 
   $mainDir."\\\\\\".$subDir, 
  ".\\".$mainDir."\\..\\".$mainDir."\\".$subDir,
  "BADDIR",  
  
  // unixifed path
  $unixifiedDir,
);

for($i = 0; $i<count($allDirs); $i++) {
  $j = $i+1;
  $dir = $allDirs[$i];
  echo "\n-- Iteration $j --\n";
  touch($absSubDir."\\".$fromFile);
  $res = rename($dir."\\".$fromFile, $dir."\\".$toFile);
  var_dump($res);
  if ($res == true) {
     $res = rename($dir."\\".$toFile, $dir."\\".$fromFile);
     var_dump($res);
  }
  unlink($absSubDir."\\".$fromFile);
}

chdir($old_dir_path);
rmdir($absSubDir);
rmdir($absMainDir);

echo "\n*** Done ***\n";
?>