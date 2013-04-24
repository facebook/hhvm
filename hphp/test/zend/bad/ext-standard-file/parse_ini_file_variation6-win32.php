<?php
/* Prototype  : array parse_ini_file(string filename [, bool process_sections])
 * Description: Parse configuration file 
 * Source code: ext/standard/basic_functions.c
 * Alias to functions: 
 */

echo "*** Testing parse_ini_file() : variation ***\n";
$mainDir = "parseIniFileVar6.dir";
$subDir = "parseIniFileVar6Sub";
$absMainDir = dirname(__FILE__)."\\".$mainDir;
mkdir($absMainDir);
$absSubDir = $absMainDir."\\".$subDir;
mkdir($absSubDir);

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

$filename = 'ParseIniFileVar6.ini';
$content="a=test";
$absFile = $absSubDir.'/'.$filename;
$h = fopen($absFile,"w");
fwrite($h, $content);
fclose($h);

for($i = 0; $i<count($allDirs); $i++) {
  $j = $i+1;
  $dir = $allDirs[$i];
  echo "\n-- Iteration $j --\n";
  var_dump(parse_ini_file($dir."\\".$filename));
}

unlink($absFile);
chdir($old_dir_path);
rmdir($absSubDir);
rmdir($absMainDir);

echo "\n*** Done ***\n";
?>