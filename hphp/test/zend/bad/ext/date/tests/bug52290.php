<?php
$tz = 'UTC';
date_default_timezone_set($tz);

$ts = strtotime('2006-01-01');
$dt = new DateTime('@'.$ts);
$dt->setTimezone(new DateTimeZone($tz));

var_dump($dt->format('o-\WW-N | Y-m-d | H:i:s | U'));

$dt->setISODate(2005, 52, 1);
var_dump($dt->format('o-\WW-N | Y-m-d | H:i:s | U'));

$dt->setDate(2007, 10, 10);
var_dump($dt->format('o-\WW-N | Y-m-d | H:i:s | U'));

$dt->setTime(20, 30, 40);
var_dump($dt->format('o-\WW-N | Y-m-d | H:i:s | U'));
?>