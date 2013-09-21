<?php
$f = "gzseek_variation7.gz";
$h = gzopen($f, 'w'); 
$str1 = "This is the first line.";
$str2 = "This is the second line.";
gzwrite($h, $str1);
echo "tell=";
var_dump(gztell($h));

//seek to the end which is not sensible of course.
echo "move to the end of the file\n";
var_dump(gzseek($h, 0, SEEK_END));
echo "tell=";
var_dump(gztell($h));
gzwrite($h, $str2);
echo "tell=";
var_dump(gztell($h));
gzclose($h);
echo "\nreading the output file\n";
$h = gzopen($f, 'r');
gzpassthru($h);
gzclose($h);
echo "\n";
unlink($f);
?>
===DONE===