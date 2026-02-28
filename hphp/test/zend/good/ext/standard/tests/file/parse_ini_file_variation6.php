<?hh
/* Prototype  : array parse_ini_file(string filename [, bool process_sections])
 * Description: Parse configuration file 
 * Source code: ext/standard/basic_functions.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing parse_ini_file() : variation ***\n";
chdir(sys_get_temp_dir());
$mainDir = "parseIniFileVar6.dir";
$subDir = "parseIniFileVar6Sub";
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
  var_dump(parse_ini_file($dir."/".$filename));
}

unlink($absFile);
rmdir($absSubDir);
rmdir($absMainDir);

echo "\n*** Done ***\n";
}
