<?php
$filename = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'lchgrp.txt';
$symlink = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'lchgrp_basic_symlink.txt';

$gid = posix_getgid();

var_dump( touch( $filename ) );
var_dump( symlink( $filename, $symlink ) );
var_dump( lchgrp( $filename, $gid ) );
var_dump( filegroup( $symlink ) === $gid );

?>
===DONE===
<?php

$filename = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'lchgrp.txt';
$symlink = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'lchgrp_basic_symlink.txt';
unlink($filename);
unlink($symlink);

?>