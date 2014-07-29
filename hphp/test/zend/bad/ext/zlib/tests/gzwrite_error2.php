<?php

$filename = "gzwrite_error2.txt.gz";
$h = gzopen($filename, 'w');
$str = "Here is the string to be written. ";
var_dump(gzwrite( $h, $str, 0 ) );
var_dump(gzwrite( $h, $str, -1 ) );
gzclose($h);

$h = gzopen($filename, 'r');
gzpassthru($h);
gzclose($h);
echo "\n";
unlink($filename);
?>
===DONE===