<?hh
/* Prototype  : int file_put_contents(string file, mixed data [, int flags [, resource context]])
 * Description: Write/Create a file with contents data and return the number of bytes written
 * Source code: ext/standard/file.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing file_put_contents() : usage variation ***\n";
chdir(sys_get_temp_dir());
$mainDir = "filePutContentsVar7.dir";
$subDir = "filePutContentsVar7Sub";
$absMainDir = sys_get_temp_dir().'/'.$mainDir;
mkdir($absMainDir);
$absSubDir = $absMainDir."/".$subDir;
mkdir($absSubDir);





// Note invalid dirs in p8 result in (The system cannot find the path specified.)
// rather than No Such File or Directory in php.net
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
$data = "This was the written data";

for($i = 0; $i<count($allDirs); $i++) {
  $j = $i+1;
  $dir = $allDirs[$i];
  echo "\n-- Iteration $j --\n";
  $res = file_put_contents($dir."/".$filename, HH\Lib\Legacy_FIXME\cast_for_arithmetic($data) + $i);
  if ($res !== false) {
      $in = file_get_contents($absFile);
      if (HH\Lib\Legacy_FIXME\eq($in, HH\Lib\Legacy_FIXME\cast_for_arithmetic($data) + $i)) {
         echo "Data written correctly\n";
      }
      else {
         echo "Data not written correctly or to correct place\n";
      }
      unlink($dir."/".$filename);
  }
  else {
     echo "No data written\n";
  }

}

rmdir($absSubDir);
rmdir($absMainDir);

echo "\n*** Done ***\n";
}
