<?php


echo "*** Test by calling method or function with its expected arguments ***\n";

$gid = posix_getgid();
var_dump(posix_setgid( $gid ) );


?>
===DONE===