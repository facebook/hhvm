<?php
/* Prototype: bool is_file ( string $filename );
   Description: Tells whether the filename is a regular file
                Returns TRUE if the filename exists and is a regular file
*/

echo "*** Testing is_file() error conditions ***";
$file_path = dirname(__FILE__);
var_dump( is_file() );  // Zero No. of args

/* no of args > expected */
$file_handle = fopen($file_path."/is_file_error.tmp", "w");
var_dump( is_file( $file_path."/is_file_error.tmp", $file_path."/is_file_error1.tmp") );

/* Non-existing file */
var_dump( is_file($file_path."/is_file_error1.tmp") );

/* Passing resource as an argument */
var_dump( is_file($file_handle) );

fclose($file_handle);

echo "\n*** Done ***";
?>
<?php
$file_path = dirname(__FILE__);
if(file_exists($file_path."/is_file_error.tmp")) {
  unlink($file_path."/is_file_error.tmp");
}
if(file_exists($file_path."/is_file_error1.tmp")) {
  unlink($file_path."/is_file_error1.tmp");
}
?>