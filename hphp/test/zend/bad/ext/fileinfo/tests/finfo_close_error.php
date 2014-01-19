<?php
/* Prototype  : resource finfo_close(resource finfo)
 * Description: Close fileinfo resource. 
 * Source code: ext/fileinfo/fileinfo.c
 * Alias to functions: 
 */

echo "*** Testing finfo_close() : error conditions ***\n";

$magicFile = dirname( __FILE__ ) . DIRECTORY_SEPARATOR . 'magic';
$finfo = finfo_open( FILEINFO_MIME, $magicFile );
$fp = fopen( __FILE__, 'r' );

echo "\n-- Testing finfo_close() function with Zero arguments --\n";
var_dump( finfo_close() );

echo "\n-- Testing finfo_close() function with more than expected no. of arguments --\n";
var_dump( finfo_close( $finfo, '10') );

echo "\n-- Testing finfo_close() function with wrong resource type --\n";
var_dump( finfo_close( $fp ) );

?>
===DONE===