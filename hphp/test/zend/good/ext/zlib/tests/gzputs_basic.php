<?php

$filename = dirname(__FILE__)."/gzputs_basic.txt.gz";
$h = gzopen($filename, 'w');
$str = "Here is the string to be written. ";
$length = 10;
var_dump(gzputs( $h, $str ) );
var_dump(gzputs( $h, $str, $length ) );
gzclose($h);

$h = gzopen($filename, 'r');
gzpassthru($h);
gzclose($h);
echo "\n";
unlink($filename);
?>
===DONE===