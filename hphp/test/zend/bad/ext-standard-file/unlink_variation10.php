<?php
/* Prototype  : bool unlink(string filename[, context context])
 * Description: Delete a file 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */

echo "*** Testing unlink() : variation ***\n";

$workDir = "unlinkVar8.tmp";
$tmpDir = "subDir.tmp";
$dirToLinkTo = $workDir.'/'."linkme.tmp";

mkdir($workDir);
$cwd = getcwd();
mkdir($dirToLinkTo);

$dirs = array(
             // relative
             $workDir.'/'.$tmpDir,
             './'.$workDir.'/'.$tmpDir,
             $workDir.'/../'.$workDir.'/'.$tmpDir,
                         
             //absolute
             $cwd.'/'.$workDir.'/'.$tmpDir,
             $cwd.'/./'.$workDir.'/'.$tmpDir,
             $cwd.'/'.$workDir.'/../'.$workDir.'/'.$tmpDir,
           
             // multiple separators
             $workDir.'//'.$tmpDir,
             $cwd.'//'.$workDir.'//'.$tmpDir,
             
             );
             

foreach($dirs as $dirToUnlink) {
   test_link($workDir.'/'.$tmpDir, $dirToLinkTo, $dirToUnlink, true);  //soft link
   //cannot test hard links unless you are root.
}

echo "\n--- try to unlink a directory ---\n";
unlink($dirToLinkTo);
rmdir($dirToLinkTo);
rmdir($workDir);

function test_link($linkedDir, $toLinkTo, $tounlink, $softlink) {
   if ($softlink == true) {
   	  symlink($toLinkTo, $linkedDir);
   	  $msg = "soft link";
   }
   else {
   	  link($toLinkTo, $linkedDir);
   	  $msg = "hard link";   	  
   }   
   echo "-- unlinking $msg $tounlink --\n";           
   $res = unlink($tounlink);
   if ($res === true) {
      if (is_link($tounlink) === false) {
      	echo "directory unlinked\n";
      }
      else {
        echo "FAILED: directory not unlinked\n";
      }
   }
   else {
      unlink($linkedDir);
   }
}


?>
===DONE===