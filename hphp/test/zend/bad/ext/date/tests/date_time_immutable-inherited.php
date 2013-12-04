<?php
$tz = new DateTimeZone("Asia/Tokyo");
$current = "2012-12-27 16:24:08";

echo "\ngetTimezone():\n";
$v = date_create_immutable($current);
$x = $v->getTimezone();
var_dump($x->getName());

echo "\ngetTimestamp():\n";
$v = date_create_immutable($current);
$x = $v->getTimestamp();
var_dump($x);
?>