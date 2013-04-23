<?php
$original = 'aaaaaaaaaaaaaaa';
$packed=gzdeflate($original);
echo strlen($packed)." ".strlen($original)."\n";
$unpacked=gzinflate($packed, strlen($original));
if (strcmp($original,$unpacked)==0) echo "Strings are equal\n";

$unpacked=gzinflate($packed, strlen($original)*10);
if (strcmp($original,$unpacked)==0) echo "Strings are equal\n";

$unpacked=gzinflate($packed, 1);
if ($unpacked === false) echo "Failed (as expected)\n";
?>