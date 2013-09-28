<?php

$f = dirname(__FILE__)."/004.txt.gz";
$h = gzopen($f, 'r');
$extra_arg = 'nothing';
var_dump(gzgetc( $h, $extra_arg ) );

var_dump(gzgetc() );

gzclose($h);
?>
===DONE===