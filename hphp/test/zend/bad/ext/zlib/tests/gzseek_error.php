<?php
$f = dirname(__FILE__)."/004.txt.gz";
$h = gzopen($f, 'r');
$offset = 1;
$whence = SEEK_SET;
$extra_arg = 'nothing'; 

var_dump(gzseek( $h, $offset, $whence, $extra_arg ) );
var_dump(gzseek($h));
var_dump(gzseek());

?>
===DONE===