<?php
/*
 *  Prototype: float disk_total_space( string directory )
 *  Description: given a string containing a directory, this function 
 *               will return the total number of bytes on the corresponding 
 *               filesystem or disk partition.
 */

$file_path = dirname(__FILE__);

echo "*** Testing with a directory ***\n";
var_dump( disk_total_space($file_path."/..") ); 

echo "\nTesting for the return type ***\n";
$return_value = disk_total_space($file_path); 
var_dump( is_float($return_value) );

echo "\n*** Testing with different directory combinations ***";
$dir = "/disk_total_space";

mkdir($file_path.$dir);

$dirs_arr = array(
  ".",
  $file_path.$dir,
  $file_path."/.".$dir,

  /* Testing a file trailing slash */
  $file_path."".$dir."/",
  $file_path."/.".$dir."/",

  /* Testing file with double trailing slashes */
  $file_path.$dir."//",
  $file_path."/.".$dir."//",
  $file_path."/./".$dir."//",

  /* Testing Binary safe */
  $file_path.$dir.chr(0),
  $file_path."/.".$dir.chr(0),
  ".".chr(0).$file_path.$dir,
  ".".chr(0).$file_path.$dir.chr(0)
);


$count = 1;
/* loop through to test each element the above array */
foreach($dirs_arr as $dir1) {
  echo "\n-- Iteration $count --\n";
  var_dump( disk_total_space( $dir1 ) );
  $count++;
}

echo "*** Testing with Binary Input ***\n";
var_dump( disk_total_space(b"$file_path") ); 

echo"\n--- Done ---";
?>

<?php
$file_path = dirname(__FILE__);
rmdir($file_path."/disk_total_space");
?>

