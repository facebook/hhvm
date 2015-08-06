<?php

$s = socket_create(AF_UNIX, SOCK_STREAM, 0);
var_dump($s);

$f = '/tmp/socktest'.rand();
$ret = socket_bind($s, $f);
var_dump($ret);

$ret = socket_getsockname($s, $n);
var_dump($ret);

var_dump($f);
var_dump($n);
var_dump($f === $n);

socket_close($s);
unlink($f);
