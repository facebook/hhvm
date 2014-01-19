<?php


echo "*** Test by calling method or function with incorrect numbers of arguments ***\n";

$fd = 'foo';
$extra_arg = 'bar'; 

var_dump(posix_ttyname( $fd, $extra_arg ) );

var_dump(posix_ttyname(  ) );


?>