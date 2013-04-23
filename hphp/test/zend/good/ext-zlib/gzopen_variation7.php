<?php

$f = dirname(__FILE__)."/004.txt.gz";
$h1 = gzopen($f, 'r');
$h2 = gzopen($f, 'r');

var_dump(gzread($h1, 30));
var_dump(gzread($h2, 10));
var_dump(gzread($h1, 15));
gzclose($h1);
var_dump(gzread($h2, 50));
// deliberately do not close $h2
?>
===DONE===