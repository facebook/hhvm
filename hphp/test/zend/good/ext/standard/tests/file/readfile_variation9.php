<?hh
/* Prototype  : int readfile(string filename [, bool use_include_path[, resource context]])
 * Description: Output a file or a URL 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing readfile() : variation ***\n";
$mainDir = 'readfile_variation9';
$subDir = 'readfile_variation9_sub';
$absMainDir = sys_get_temp_dir().'/'.$mainDir;
mkdir($absMainDir);
$absSubDir = $absMainDir."/".$subDir;
mkdir($absSubDir);

$theFile = "fileToRead.tmp";
$absFile = $absSubDir.'/'.$theFile;

// create the file
$h = fopen($absFile,"w");
fwrite($h, "The File Contents");
fclose($h);



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
  $ok = readfile($dir.'/'.$theFile);
  if ($ok === 1) {
     echo "\n";
  }
}

unlink($absFile);
rmdir($absSubDir);
rmdir($absMainDir);

echo "\n*** Done ***\n";
}
