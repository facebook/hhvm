<?php
/* Prototype  : resource finfo_open([int options [, string arg]])
 * Description: Create a new fileinfo resource. 
 * Source code: ext/fileinfo/fileinfo.c
 * Alias to functions: 
 */

$magicFile = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'magic';

echo "*** Testing finfo_open() : error functionality ***\n";

var_dump( finfo_open( FILEINFO_MIME, 'foobarfile' ) );
var_dump( finfo_open( array(), $magicFile ) );
var_dump( finfo_open( FILEINFO_MIME, $magicFile, 'extraArg' ) );
var_dump( finfo_open( PHP_INT_MAX - 1, $magicFile ) );
var_dump( finfo_open( 'foobar' ) );

var_dump( new finfo('foobar') );

?>
===DONE===