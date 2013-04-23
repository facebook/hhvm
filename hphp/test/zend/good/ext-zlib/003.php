<?php
$original = str_repeat("hallo php",4096);
$packed = gzencode($original);
echo strlen($packed)." ".strlen($original). "\n";
if (strcmp($original, gzdecode($packed)) == 0) echo "Strings are equal";
?>