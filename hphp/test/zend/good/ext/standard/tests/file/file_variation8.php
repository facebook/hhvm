<?hh
/* Prototype  : array file(string filename [, int flags[, resource context]])
 * Description: Read entire file into an array 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing file() : variation ***\n";
chdir(sys_get_temp_dir());
$mainDir = "fileVar8.dir";
$subDir = "fileVar8Sub";
$absMainDir = sys_get_temp_dir().'/'.$mainDir;
mkdir($absMainDir);
$absSubDir = $absMainDir."/".$subDir;
mkdir($absSubDir);



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

$filename = 'FileGetContentsVar7.tmp';
$absFile = $absSubDir.'/'.$filename;
$h = fopen($absFile,"w");
fwrite($h, "contents read");
fclose($h);

for($i = 0; $i<count($allDirs); $i++) {
  $j = $i+1;
  $dir = $allDirs[$i];
  echo "\n-- Iteration $j --\n";
  var_dump(file($dir."/".$filename));
}

unlink($absFile);
rmdir($absSubDir);
rmdir($absMainDir);

echo "\n*** Done ***\n";
}
