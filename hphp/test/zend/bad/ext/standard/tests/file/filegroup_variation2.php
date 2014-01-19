<?php
/* Prototype: int filegroup ( string $filename )
 * Description: Returns the group ID of the file, or FALSE in case of an error.
 */

/* Testing filegroup() with invalid arguments -int, float, bool, NULL, resource */

$file_path = dirname(__FILE__);
$file_handle = fopen($file_path."/filegroup_variation2.tmp", "w");

echo "*** Testing Invalid file types ***\n";
$filenames = array(
  /* Invalid filenames */
  -2.34555,
  " ",
  "",
  TRUE,
  FALSE,
  NULL,
  $file_handle,
  
  /* scalars */
  1234,
  0
);
   
/* loop through to test each element the above array */
foreach( $filenames as $filename ) {
  var_dump( filegroup($filename) );
  clearstatcache();
}
fclose($file_handle);

echo "\n*** Done ***";
?>
<?php
$file_path = dirname(__FILE__);
unlink($file_path."/filegroup_variation2.tmp");
?>