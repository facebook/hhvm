<?php
/* 
Prototype: int fileinode ( string $filename );
Description: Returns the inode number of the file, or FALSE in case of an error.
*/

/* Creating soft and hard links to a file and applying fileinode() on links */ 

$file_path = dirname(__FILE__);
fclose( fopen($file_path."/fileinode_variation1.tmp", "w") );

echo "*** Testing fileinode() with links ***\n";
/* With symlink */
symlink($file_path."/fileinode_variation1.tmp", $file_path."/fileinode_variation1_symlink.tmp");
var_dump( fileinode($file_path."/fileinode_variation1_symlink.tmp") ); //expected true
clearstatcache();

/* With hardlink */
link($file_path."/fileinode_variation1.tmp", $file_path."/fileinode_variation1_link.tmp");
var_dump( fileinode($file_path."/fileinode_variation1_link.tmp") );  // expected: true
clearstatcache();

echo "\n*** Done ***";
?>
<?php
$file_path = dirname(__FILE__);
unlink($file_path."/fileinode_variation1_symlink.tmp");
unlink($file_path."/fileinode_variation1_link.tmp");
unlink($file_path."/fileinode_variation1.tmp");
?>
