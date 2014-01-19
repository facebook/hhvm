<?php

$f = dirname(__FILE__)."/004.txt.gz";
$h = gzopen($f, 'r');
$length = 10;
$extra_arg = 'nothing';
var_dump(gzgets( $h, $length, $extra_arg ) );

var_dump(gzgets());


?>
===DONE===