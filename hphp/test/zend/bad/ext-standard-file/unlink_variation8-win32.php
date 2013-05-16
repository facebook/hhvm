<?php
/* Prototype  : bool unlink(string filename[, context context])
 * Description: Delete a file 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */

echo "*** Testing unlink() : variation ***\n";

$workDir = "unlinkVar8.tmp";
$tmpFile = "file.tmp";
chdir(__DIR__);
mkdir($workDir);
$cwd = __DIR__;

$files = array(
             // relative
             $workDir.'/'.$tmpFile,
             './'.$workDir.'/'.$tmpFile,
             $workDir.'/../'.$workDir.'/'.$tmpFile,
             
             // relative bad path
             $workDir.'/../BADDIR/'.$tmpFile,
             'BADDIR/'.$tmpFile,
             
             //absolute
             $cwd.'/'.$workDir.'/'.$tmpFile,
             $cwd.'/./'.$workDir.'/'.$tmpFile,
             $cwd.'/'.$workDir.'/../'.$workDir.'/'.$tmpFile,

             //absolute bad path             
             $cwd.'/BADDIR/'.$tmpFile,
             
             //trailing separators
             $workDir.'/'.$tmpFile.'/',
             $cwd.'/'.$workDir.'/'.$tmpFile.'/',
             
             // multiple separators
             $workDir.'//'.$tmpFile,
             $cwd.'//'.$workDir.'//'.$tmpFile,
             
             );
             

foreach($files as $fileToUnlink) {
   test_realfile($workDir.'/'.$tmpFile, $fileToUnlink);
}

rmdir($workDir);

function test_realfile($file, $tounlink) {
   touch($file);
   echo "-- removing $tounlink --\n";           
   $res = unlink($tounlink);
   if ($res === true) {
      if (file_exists($tounlink) === false) {
      	echo "file removed\n";
      }
      else {
        echo "FAILED: file not removed\n";
      }
   }
   else {
      unlink($file);
   }
}
?>
===DONE===