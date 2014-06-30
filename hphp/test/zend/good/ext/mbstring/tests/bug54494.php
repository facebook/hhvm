<?php

//declare(encoding = 'UTF-8');
mb_internal_encoding('UTF-8');

header('Content-Type: text/plain; charset=UTF-32LE');

$stringOr = "hällö wörld\n";

$mode = "UTF-32LE";

echo "$mode:\n";

$string = mb_convert_encoding($stringOr, $mode);
$length = mb_strlen($string, $mode);
echo "Length: ", $length, "\n";


for ($i=0; $i < $length; $i++) {
  $t = unpack("H*",mb_substr($string, $i, 1, $mode));
  echo $t[1];
}
echo "\n";


$mode = "UCS-2LE";

echo "$mode:\n";

$string = mb_convert_encoding($stringOr, $mode);
$length = mb_strlen($string, $mode);
echo "Length: ", $length, "\n";


for ($i=0; $i < $length; $i++) {
  $t = unpack("H*",mb_substr($string, $i, 1, $mode));
  echo $t[1];
}
echo "\n";