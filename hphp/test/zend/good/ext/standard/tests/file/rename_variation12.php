<?hh
/* Prototype  : bool rename(string old_name, string new_name[, resource context])
 * Description: Rename a file 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */

/* Creating unique files in various dirs by passing relative paths to $dir arg */
<<__EntryPoint>> function main(): void {
echo "*** Testing rename() with absolute and relative paths ***\n";
$mainDir = "renameVar11";
$subDir = "renameVar11Sub";

$absMainDir = sys_get_temp_dir().'/'.$mainDir;
mkdir($absMainDir);
$absSubDir = $absMainDir."/".$subDir;
mkdir($absSubDir);

$fromFile = "renameMe.tmp";
$toFile = "IwasRenamed.tmp";

$old_dir_path = getcwd();
chdir(sys_get_temp_dir());

$allDirs = vec[
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
];

for($i = 0; $i<count($allDirs); $i++) {
  $j = $i+1;
  $dir = $allDirs[$i];
  echo "\n-- Iteration $j --\n";
  touch($absSubDir."/".$fromFile);
  $res = rename($dir."/".$fromFile, $dir."/".$toFile);
  var_dump($res);
  if ($res == true) {
     $res = rename($dir."/".$toFile, $dir."/".$fromFile);
     var_dump($res);
  }
  unlink($absSubDir."/".$fromFile);
}

chdir($old_dir_path);
rmdir($absSubDir);
rmdir($absMainDir);

echo "\n*** Done ***\n";
}
