<?php
/* Prototype  : resource fopen(string filename, string mode [, bool use_include_path [, resource context]])
 * Description: Open a file or a URL and return a file pointer 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */

echo "*** Testing fopen() : variation ***\n";

// fopen with interesting windows paths.
$testdir = dirname(__FILE__).'/fopen10.tmpDir';
$rootdir = 'fopen10.tmpdirTwo';
mkdir($testdir);
mkdir('c:\\'.$rootdir);

$unixifiedDir = '/'.substr(str_replace('\\','/',$testdir),3);

$paths = array('c:\\', 
               'c:', 
               'c', 
               '\\', 
               '/', 
               'c:'.$rootdir, 
               'c:adir', 
               'c:\\/', 
               'c:\\'.$rootdir.'\\/',
               'c:\\'.$rootdir.'\\', 
               'c:\\'.$rootdir.'/',
               $unixifiedDir, 
               '/sortout');

$file = "fopen_variation10.tmp";
$firstfile = 'c:\\'.$rootdir.'\\'.$file;
$secondfile = $testdir.'\\'.$file;
$thirdfile = 'c:\\'.$file;

$h = fopen($firstfile, 'w');
fwrite($h, "file in $rootdir");
fclose($h);

$h = fopen($secondfile, 'w');
fwrite($h, "file in fopen10.tmpDir");
fclose($h);

$h = fopen($thirdfile, 'w');
fwrite($h, "file in root");
fclose($h);

foreach($paths as $path) {
      echo "\n--$path--\n";
      $toFind = $path.'\\'.$file;
         $h = fopen($toFind, 'r');
         if ($h === false) {
            echo "file not opened for read\n";
         }
         else {
            fpassthru($h);
            echo "\n";
         }
         fclose($h);
};

unlink($firstfile);
unlink($secondfile);
unlink($thirdfile);
rmdir($testdir);
rmdir('c:\\'.$rootdir);


?>
===DONE===