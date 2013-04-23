<?php

$filename = dirname(__FILE__)."/004.txt.gz";
$h = gzopen($filename, 'r');
$str = b"Here is the string to be written. ";
$length = 10;
var_dump(gzwrite( $h, $str ) );
var_dump(gzread($h, 10));
var_dump(gzwrite( $h, $str, $length ) );
gzclose($h);

?>
===DONE===