<?php
$filename = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'lchgrp.txt';
$symlink = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'symlink.txt';

$gid = posix_getgid();

var_dump( touch( $filename ) );
var_dump( symlink( $filename, $symlink ) );
var_dump( lchgrp( $filename, $gid ) );
var_dump( filegroup( $symlink ) === $gid );

?>
===DONE===
<?php error_reporting(0); ?>
<?php

$filename = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'lchgrp.txt';
$symlink = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'symlink.txt';
unlink($filename);
unlink($symlink);

?>