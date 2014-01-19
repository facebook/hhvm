<?php
ini_set('default_socket_timeout', 2);


$dir = 'ftp://your:self@localhost/';

var_dump(opendir($dir));
var_dump(opendir($dir));

?>