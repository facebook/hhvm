<?php
$f = "temp2.txt.gz";
$h = gzopen($f, 'w');
gzwrite($h, b'The first string.');
var_dump(gzrewind($h));
gzwrite($h, b'The second string.');
gzclose($h);

$h = gzopen($f, 'r');
gzpassthru($h);
gzclose($h);
unlink($f);
echo "\n";
?>
===DONE===