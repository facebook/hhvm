<?php
$f = dirname(__FILE__)."/004.txt.gz";
$h = gzopen($f, 'r');
echo "test rewind before doing anything\n";
var_dump(gzrewind($h));
var_dump(gztell($h));
echo "\nfirst 30 characters=".gzread($h, 30)."\n";
var_dump(gztell($h));
gzrewind($h);
var_dump(gztell($h));
echo "first 10 characters=".gzread($h, 10)."\n";
gzrewind($h);
echo "first 20 characters=".gzread($h, 20)."\n";
gzclose($h);
?>
===DONE===