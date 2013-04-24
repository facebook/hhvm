<?php
/* Prototype  : resource fopen(string filename, string mode [, bool use_include_path [, resource context]])
 * Description: Open a file or a URL and return a file pointer 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */

echo "*** Testing fopen() : variation ***\n";

// fopen with interesting windows paths.
$testDir = 'fopen14.tmpDir';
$absTestDir = getcwd().'/'.$testDir;
$file = "fopen_variation14.tmp";
$absFile = $absTestDir.'/'.$file;

mkdir($testDir);

$files = array("file://$testDir/$file",
               "file://./$testDir/$file",
               "file://$absTestDir/$file"
);

runtest($files);

chdir($testDir);
$files = array("file://../$testDir/$file",
               "file://$absTestDir/$file",
);
runtest($files);
chdir("..");
rmdir($testDir);

function runtest($fileURIs) {
   global $absFile;
   $iteration = 0;
   foreach($fileURIs as $fileURI) {
      echo "--- READ: $fileURI ---\n";
   
      $readData = "read:$iteration";
      $writeData = "write:$iteration";
      
      // create the file and test read
      $h = fopen($absFile, 'w');
      fwrite($h, $readData);
      fclose($h);
      
      $h = fopen($fileURI, 'r');
      if ($h !== false) {
         if (fread($h, 4096) != $readData) {
            echo "contents not correct\n";
         }
         else {
            echo "test passed\n";
         }
         fclose($h);
      }
      unlink($absFile);
      
      echo "--- WRITE: $fileURI ---\n";   
      // create the file to test write
      $h = fopen($fileURI, 'w');
      if ($h !== false) {
	      fwrite($h, $writeData);
	      fclose($h);
	      
	      $h = fopen($absFile, 'r');
	      if ($h !== false) {
	         if (fread($h, 4096) != $writeData) {
	            echo "contents not correct\n";
	         }
	         else {
	            echo "test passed\n";
	         }
	         fclose($h);
	      }
	      unlink($absFile);
	   }
   }
}


?>
===DONE===