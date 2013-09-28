<?php
$n = 1;
L1:
echo "$n: ok\n";
$n++;
if ($n <= 3) goto L1;
?>