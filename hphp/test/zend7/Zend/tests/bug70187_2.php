<?php
$a = 1;
unset($a);
unserialize(serialize($GLOBALS));
echo "ok\n";
?>
