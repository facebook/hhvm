<?php
/* Prototype  : bool lchown (string filename, mixed user)
 * Description: Change file owner of a symlink
 * Source code: ext/standard/filestat.c
 * Alias to functions: 
 */

echo "*** Testing lchown() : error functionality ***\n";

// Set up
$filename = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'lchown.txt';
touch( $filename );
$uid = posix_getuid();


// Less than expected arguments
var_dump( lchown( $filename ) );

// More than expected arguments
var_dump( lchown( $filename, $uid, 'foobar' ) );

// Non-existent filename
var_dump( lchown( 'foobar_lchown.txt', $uid ) );

// Wrong argument types
var_dump( lchown( new StdClass(), $uid ) );
var_dump( lchown( array(), $uid ) );

// Bad user
var_dump( lchown( $filename, -5 ) );

?>
===DONE===
<?php

$filename = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'lchown.txt';
unlink($filename);

?>