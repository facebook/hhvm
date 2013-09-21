<?php
/* Prototype  : bool unlink(string filename[, context context])
 * Description: Delete a file 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */

echo "*** Testing unlink() : variation ***\n";

$workDir = "unlinkVar8.tmp";
$tmpFile = "file.tmp";
$fileToLinkTo = $workDir.'/'."linkme.tmp";

mkdir($workDir);
$cwd = getcwd();
touch($fileToLinkTo);

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
   test_link($workDir.'/'.$tmpFile, $fileToLinkTo, $fileToUnlink, true);  //soft link
   test_link($workDir.'/'.$tmpFile, $fileToLinkTo, $fileToUnlink, false); //hard link   
}

unlink($fileToLinkTo);
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

function test_link($linkedfile, $toLinkTo, $tounlink, $softlink) {
   if ($softlink == true) {
   	  symlink($toLinkTo, $linkedfile);
   	  $msg = "soft link";
   }
   else {
   	  link($toLinkTo, $linkedfile);
   	  $msg = "hard link";   	  
   }   
   echo "-- unlinking $msg $tounlink --\n";           
   $res = unlink($tounlink);
   if ($res === true) {
      if (file_exists($tounlink) === false) {
      	echo "file unlinked\n";
      }
      else {
        echo "FAILED: file not unlinked\n";
      }
   }
   else {
      unlink($linkedfile);
   }
}


?>
===DONE===