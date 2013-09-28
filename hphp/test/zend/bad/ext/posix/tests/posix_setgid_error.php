<?php


echo "*** Test by calling method or function with incorrect numbers of arguments ***\n";

$gid = posix_getgid();
$extra_arg = '123';

var_dump(posix_setgid( $gid, $extra_arg ) );
var_dump(posix_setgid(  ) );

?>
===DONE===