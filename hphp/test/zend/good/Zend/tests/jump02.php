<?php
$n = 1;
L1:
if ($n > 3) goto L2;
echo "$n: ok\n";
$n++;
goto L1;
L2:
?>