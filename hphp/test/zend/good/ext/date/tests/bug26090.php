<?php
$t = '2003-10-28 10:20:30-0800';
echo date('Y-m-d H:i:s T', strtotime($t)) . "\n";

$t = '2003-10-28 10:20:30-08:00';
echo date('Y-m-d H:i:s T', strtotime($t)) . "\n";
?>