<?php
/*
 *  Prototype: float disk_free_space( string directory )
 *  Description: Given a string containing a directory, this function 
 *               will return the number of bytes available on the corresponding
 *               filesystem or disk partition
 */

$file_path = dirname(__FILE__);

echo "*** Testing with a directory ***\n";
var_dump( disk_free_space($file_path."/..") ); 
var_dump( diskfreespace($file_path."/..") ); 

echo "\nTesting for the return type ***\n";
$return_value = disk_free_space($file_path); 
var_dump( is_float($return_value) );

echo "\n*** Testing with different directory combinations ***";
$dir = "/disk_free_space";
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
  var_dump( disk_free_space( $dir1 ) );
  var_dump( diskfreespace( $dir1 ) );
  $count++;
}

echo"\n--- Done ---";
?>
<?php
$file_path = dirname(__FILE__);
rmdir($file_path."/disk_free_space");
?>

