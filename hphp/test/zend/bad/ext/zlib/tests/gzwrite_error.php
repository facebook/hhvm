<?php
$filename = "gzwrite_error.txt.gz";
$h = gzopen($filename, 'w');
$str = "Here is the string to be written. ";
$length = 10;
$extra_arg = 'nothing'; 
var_dump(gzwrite($h, $str, $length, $extra_arg));
var_dump(gzwrite($h));
var_dump(gzwrite());

gzclose($h);
unlink($filename);

?>
===DONE===