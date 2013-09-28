<?php

$filename = "zlib_wrapper_fflush_basic.txt.gz";
$h = gzopen($filename, 'w');
$str = "Here is the string to be written.";
$length = 10;
var_dump(fflush($h));
gzwrite( $h, $str);
gzwrite( $h, $str);
var_dump(fflush($h));
gzclose($h);

$h = gzopen($filename, 'r');
gzpassthru($h);
gzclose($h);
echo "\n";
unlink($filename);
?>
===DONE===