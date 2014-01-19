<?php

$f = dirname(__FILE__)."/004.txt.gz";
$h = gzopen($f, 'r');
$length = 10; 
$extra_arg = 'nothing'; 

var_dump(gzread( $h, $length, $extra_arg ) );

var_dump(gzread());

gzclose($h);

?>
===DONE===