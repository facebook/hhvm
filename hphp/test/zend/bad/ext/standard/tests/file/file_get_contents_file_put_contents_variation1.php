<?php
/* Prototype: string file_get_contents( string $filename[, bool $use_include_path[, 
 *                                      resource $context[, int $offset[, int $maxlen]]]] )
 * Description: Reads entire file into a string
 */

/* Prototype: int file_put_contents( string $filename, mixed $data[,int $flags[, resource $context]] )
 * Description: Write a string to a file
 */

/* Testing variation in all argument values */
$file_path = dirname(__FILE__);
include($file_path."/file.inc");

echo "*** Testing with variations in the arguments values ***\n";

$buffer_types = array("text", "numeric", "text_with_new_line", "alphanumeric");

foreach( $buffer_types as $type) {
  fill_buffer($buffer, $type, 100);
  file_put_contents( $file_path."/file_put_contents_variation1.tmp", $buffer);
  var_dump( file_get_contents($file_path."/file_put_contents_variation1.tmp", 0) );
  var_dump( file_get_contents($file_path."/file_put_contents_variation1.tmp", 1) );
  var_dump( file_get_contents($file_path."/file_put_contents_variation1.tmp", 0, NULL, 5) );
  var_dump( file_get_contents($file_path."/file_put_contents_variation1.tmp", 1, NULL, 5) );
  var_dump( file_get_contents($file_path."/file_put_contents_variation1.tmp", 0, NULL, 5, 20) );
  var_dump( file_get_contents($file_path."/file_put_contents_variation1.tmp", 1, NULL, 5, 20) );

}

echo "--- Done ---";
?>
<?php
//Deleting the temporary file 

$file_path = dirname(__FILE__);
unlink($file_path."/file_put_contents_variation1.tmp");
?>