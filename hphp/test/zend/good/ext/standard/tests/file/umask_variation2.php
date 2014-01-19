<?php
/* Prototype: int umask ( [int $mask] );
   Description: Changes the current umask
*/

$file_path = dirname(__FILE__);

/* Check umask() on file/dir */

echo "*** Testing umask() on file and directory ***\n";
// temp filename used
$filename = "$file_path/umask_variation2.tmp";
// temp dir used
$dirname = "$file_path/umask_variation2";

for($mask = 0351; $mask <= 0777; $mask++) {
  echo "-- Setting umask to ";
  echo sprintf('%03o', $mask);
  echo " --\n";
  // setting umask
  umask($mask);
 
  /* umasking file */
  // creating temp file
  $fp = fopen($filename, "w");
  fclose($fp);
  echo "File permission : ";
  // check file permission
  echo substr(sprintf('%o', fileperms($filename)), -3);
  echo "\n";
  // chmod file to 0777 to enable deletion
  chmod($filename, 0777);
  // delete temp file created here
  unlink($filename);

  /* umasking directory */
  // create temp dir
  mkdir($dirname);
  echo "Directory permission : ";
  // check $dirname permission
  echo substr(sprintf('%o', fileperms($dirname)), -3);
  echo "\n";
  // chmod 0777 to enable deletion
  chmod($dirname, 0777);
  // delete temp dir created
  rmdir($dirname);
}

echo "Done\n";
?>