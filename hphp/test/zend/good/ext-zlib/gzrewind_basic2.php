<?php
$f = dirname(__FILE__)."/004.txt.gz";
$h = gzopen($f, 'r');

// read to the end of the file
echo "read to the end of the file, then rewind\n";
gzread($h, 10000);
var_dump(gzeof($h));
var_dump(gztell($h));
gzrewind($h);
var_dump(gzeof($h));
var_dump(gztell($h));
echo "first 20 characters=".gzread($h,20)."\n";

gzclose($h);
?>
===DONE===