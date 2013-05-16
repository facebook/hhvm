<?php
$f = "gzseek_variation4.gz";
$h = gzopen($f, 'w'); 
$str1 = "This is the first line.";
$str2 = "This is the second line.";
gzwrite($h, $str1);
echo "tell=".gztell($h)."\n";

//seek forwards 20 bytes.
gzseek($h, strlen($str1) + 20, SEEK_SET);
echo "tell=".gztell($h)."\n";
gzwrite($h, $str2);
echo "tell=".gztell($h)."\n";
gzclose($h);
echo "\nreading the output file\n";
$h = gzopen($f, 'r');
echo gzread($h, strlen($str1))."\n";
echo var_dump(bin2hex(gzread($h, 20)));
echo gzread($h, strlen($str2))."\n";
gzclose($h);
unlink($f);
?>
===DONE===