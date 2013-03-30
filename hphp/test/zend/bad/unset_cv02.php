<?php
$x = "ok\n";
echo $x;
unset($GLOBALS["x"]);
echo $x;
?>