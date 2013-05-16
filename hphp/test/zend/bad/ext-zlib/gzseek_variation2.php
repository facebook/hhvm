<?php
$f = dirname(__FILE__)."/004.txt.gz";
$h = gzopen($f, 'r'); 

echo "move to the 50th byte\n";
var_dump(gzseek( $h, 50, SEEK_SET ) );
echo "tell=".gztell($h)."\n";
//read the next 10
var_dump(gzread($h, 10));

echo "\nmove forward to the 100th byte\n";
var_dump(gzseek( $h, 100, SEEK_SET ) );
echo "tell=".gztell($h)."\n";
//read the next 10
var_dump(gzread($h, 10));

echo "\nmove backward to the 20th byte\n";
var_dump(gzseek( $h, 20, SEEK_SET ) );
echo "tell=".gztell($h)."\n";
//read the next 10
var_dump(gzread($h, 10));
gzclose($h);
?>
===DONE===