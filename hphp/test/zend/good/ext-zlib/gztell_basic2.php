<?php
$f = "temp2.txt.gz";
$h = gzopen($f, 'w');
$sizes = array(7, 22, 54, 17, 27, 15, 1000);
// tell should be 7, 29, 83, 100, 127, 142, 1142

var_dump(gztell($h));
foreach ($sizes as $size) { 
   echo "bytes written=".gzwrite($h, str_repeat(b'1', $size))."\n";;
   echo "tell=".gztell($h)."\n";
}

gzclose($h);
unlink($f);
?>
===DONE===