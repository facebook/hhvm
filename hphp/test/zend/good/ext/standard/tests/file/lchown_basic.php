<?php
/* Prototype  : bool lchown (string filename, mixed user)
 * Description: Change file owner of a symlink
 * Source code: ext/standard/filestat.c
 * Alias to functions: 
 */

echo "*** Testing lchown() : basic functionality ***\n";
$filename = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'lchown_basic.txt';
$symlink = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'lchown_basic_symlink.txt';

$uid = posix_getuid();

var_dump( touch( $filename ) );
var_dump( symlink( $filename, $symlink ) );
var_dump( lchown( $filename, $uid ) );
var_dump( fileowner( $symlink ) === $uid );

?>
===DONE===
<?php

$filename = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'lchown_basic.txt';
$symlink = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'lchown_basic_symlink.txt';
unlink($filename);
unlink($symlink);

?>