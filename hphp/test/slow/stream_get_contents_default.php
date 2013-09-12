<?php

$temp = tmpfile();
$s = 'Hi, there';
fwrite($temp, $s);
fseek($temp, 0);
echo stream_get_contents($temp, -1, -1) . "\n";
fseek($temp, 0);
echo stream_get_contents($temp, 0, -1) . "\n";
fseek($temp, 0);
echo stream_get_contents($temp, 0, 0) . "\n";

fseek($temp, strlen($s) - 1);
echo stream_get_contents($temp, -1, 0) . "\n";

fclose($temp);
