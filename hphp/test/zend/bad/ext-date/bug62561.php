<?php
$tz = new DateTimeZone('America/New_York');
$ts = new DateTime('@1341115200', $tz);
$int = new DateInterval('P1D');
$dayFromTs = new DateTime('@1341115200', new DateTimeZone('America/New_York'));
$dayFromTs->add($int);

echo 'ts: '.$ts->format('Y-m-d H:i:s')."\n";
echo 'day from ts: '.$dayFromTs->format('Y-m-d H:i:s')."\n";
?>