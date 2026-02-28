<?hh
/* Prototype  : bool mkdir(string pathname [, int mode [, bool recursive [, resource context]]])
 * Description: Create a directory 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing mkdir() : variation ***\n";
chdir(sys_get_temp_dir());
$workDir = "mkdirVar5.tmp";
$subDir = "aSubDir";
mkdir($workDir);
$cwd = getcwd();

$dirs = vec[
             // relative
             $workDir.'/'.$subDir,
             './'.$workDir.'/'.$subDir,
             $workDir.'/../'.$workDir.'/'.$subDir,
             
             // relative bad path
             $workDir.'/../BADDIR/'.$subDir,
             'BADDIR/'.$subDir,
             
             //absolute
             $cwd.'/'.$workDir.'/'.$subDir,
             $cwd.'/./'.$workDir.'/'.$subDir,
             $cwd.'/'.$workDir.'/../'.$workDir.'/'.$subDir,

             //absolute bad path             
             $cwd.'/BADDIR/'.$subDir,
             
             //trailing separators
             $workDir.'/'.$subDir.'/',
             $cwd.'/'.$workDir.'/'.$subDir.'/',
             
             // multiple separators
             $workDir.'//'.$subDir,
             $cwd.'//'.$workDir.'//'.$subDir,
             
             ];
             

foreach($dirs as $dir) {
   echo "-- creating $dir --\n";           
   $res = mkdir($dir);
   if ($res === true) {
      echo "Directory created\n";
      rmdir($dir);
   }
}

rmdir($workDir);

echo "===DONE===\n";
}
