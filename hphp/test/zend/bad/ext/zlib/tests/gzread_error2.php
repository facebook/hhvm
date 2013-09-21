<?php
$f = dirname(__FILE__)."/004.txt.gz";
$h = gzopen($f, 'r');
var_dump(gzread($h, 10));
var_dump(gzread($h, 0));
var_dump(gzread($h, 5));
var_dump(gzread($h, -1));
var_dump(gzread($h, 8));
gzclose($h);

?>
===DONE===