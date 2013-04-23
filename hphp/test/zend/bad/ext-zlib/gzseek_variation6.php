<?php
$f = dirname(__FILE__)."/004.txt.gz";
$h = gzopen($f, 'r');
// move 40 bytes 
echo "move 40 bytes\n";
gzread($h, 40);
echo "tell=";
var_dump(gztell($h));
echo "move to the end\n";
var_dump(gzseek( $h, 0, SEEK_END ) );
echo "tell=";
var_dump(gztell($h));
echo "eof=";
var_dump(gzeof($h));
//read the next 10
var_dump(gzread($h, 10));
gzclose($h);
?>
===DONE===