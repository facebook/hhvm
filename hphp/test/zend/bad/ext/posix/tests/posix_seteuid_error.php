<?php


echo "*** Test by calling method or function with incorrect numbers of arguments ***\n";

$uid = '123';
$extra_arg = '12312';

var_dump(posix_seteuid( $uid, $extra_arg ) );
var_dump(posix_seteuid(  ) );


?>