<?php
/* Prototype: int fileperms ( string $filename )
 * Description: Returns the group ID of the file, or FALSE in case of an error.
 */

/* Creating soft and hard links to a file and applying fileperms() on links */ 

$file_path = dirname(__FILE__);
fclose( fopen($file_path."/fileperms_variation1.tmp", "w") );

echo "*** Testing fileperms() with links ***\n";
/* With symlink */
symlink($file_path."/fileperms_variation1.tmp", $file_path."/fileperms_variation1_symlink.tmp");
var_dump( fileperms($file_path."/fileperms_variation1_symlink.tmp") ); //expected true
clearstatcache();

/* With hardlink */
link($file_path."/fileperms_variation1.tmp", $file_path."/fileperms_variation1_link.tmp");
var_dump( fileperms($file_path."/fileperms_variation1_link.tmp") );  // expected: true
clearstatcache();

echo "\n*** Done ***";
?>
<?php
$file_path = dirname(__FILE__);
unlink($file_path."/fileperms_variation1_symlink.tmp");
unlink($file_path."/fileperms_variation1_link.tmp");
unlink($file_path."/fileperms_variation1.tmp");
?>
