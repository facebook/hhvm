<?php
date_default_timezone_set('Europe/London');

$endOfWeek = new DateTime('2010-07-27 09:46:49');
$endOfWeek->modify('this week +6 days');

echo $endOfWeek->format('Y-m-d H:i:s')."\n";
echo $endOfWeek->format('U')."\n";

/* Thar she blows! */
echo $endOfWeek->getTimestamp()."\n";

echo $endOfWeek->format('Y-m-d H:i:s')."\n";
?>