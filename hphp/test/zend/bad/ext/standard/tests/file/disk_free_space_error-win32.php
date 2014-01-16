<?php
/*
 *  Prototype: float disk_free_space( string directory )
 *  Description: Given a string containing a directory, this function will
 *               return the number of bytes available on the corresponding 
 *               filesystem or disk partition
 */

echo "*** Testing error conditions ***\n";
$file_path = dirname(__FILE__);
var_dump( disk_free_space() ); // Zero Arguments
var_dump( diskfreespace() );

var_dump( disk_free_space( $file_path, "extra argument") ); // More than valid number of arguments
var_dump( diskfreespace( $file_path, "extra argument") );


var_dump( disk_free_space( $file_path."/dir1" )); // Invalid directory
var_dump( diskfreespace( $file_path."/dir1" ));

$fh = fopen( $file_path."/disk_free_space.tmp", "w" );
fwrite( $fh, " Garbage data for the temporary file" );
var_dump( disk_free_space( $file_path."/disk_free_space.tmp" )); // file input instead of directory
var_dump( diskfreespace( $file_path."/disk_free_space.tmp" ));
fclose($fh);

echo"\n-- Done --";
?>
<?php
$file_path = dirname(__FILE__);
unlink($file_path."/disk_free_space.tmp");

?>