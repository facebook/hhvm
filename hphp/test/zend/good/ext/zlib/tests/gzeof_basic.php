<?php
// note that gzeof is an alias to gzeof. parameter checking tests will be
// the same as gzeof

$f = dirname(__FILE__)."/004.txt.gz";

echo "-- test 1 --\n";
$h = gzopen($f, 'r');
var_dump(gzeof($h));
gzpassthru($h);
var_dump(gzeof($h));
gzclose($h); 

echo "\n-- test 2 --\n";
$h = gzopen($f, 'r');
echo "reading 50 characters. eof should be false\n";
gzread($h, 50)."\n";
var_dump(gzeof($h));
echo "reading 250 characters. eof should be true\n";
gzread($h, 250)."\n";
var_dump(gzeof($h));
echo "reading 20 characters. eof should be true still\n";
gzread($h, 20)."\n";
var_dump(gzeof($h));
gzclose($h); 



?>
===DONE===