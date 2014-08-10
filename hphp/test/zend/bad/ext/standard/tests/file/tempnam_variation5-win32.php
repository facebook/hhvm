<?php
/* Prototype:  string tempnam ( string $dir, string $prefix );
   Description: Create file with unique file name.
*/

/* Passing an existing file as $prefix for tempnam() fn */

$file_path = dirname(__FILE__);

echo "*** Test tempnam() function: by passing an existing filename as prefix ***\n";
$dir_name = $file_path."/tempnam_variation6";
mkdir($dir_name);
$h = fopen($dir_name."/tempnam_variation6.tmp", "w");

for($i=1; $i<=3; $i++) {
  echo "-- Iteration $i --\n";
  $created_file = tempnam("$dir_name", "tempnam_variation6.tmp");
  
  if( file_exists($created_file) ) {
    echo "File name is => ";
    print($created_file);
    echo "\n";
  }
  else
    print("File is not created");

  unlink($created_file);
}
fclose($h);
unlink($dir_name."/tempnam_variation6.tmp");
rmdir($dir_name);

echo "\n*** Done ***\n";
?>