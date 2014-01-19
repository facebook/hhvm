<?php
date_default_timezone_set('GMT');
$start = microtime(true);
$a = mktime(1, 1, 1, 1, 1, 11111111111);
echo (microtime(true) - $start) < 1 ? "smaller than one second" : "more than a second";
?>