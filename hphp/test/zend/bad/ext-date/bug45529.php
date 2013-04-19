<?php
date_default_timezone_set('Europe/Oslo');
$tz1 = new DateTimeZone('UTC');
$tz2 = date_create('UTC')->getTimeZone();
echo $tz1->getName(), PHP_EOL;
echo $tz2->getName(), PHP_EOL;
$d = new DateTime('2008-01-01 12:00:00+0200');
$d->setTimeZone($tz1);
echo $d->format(DATE_ISO8601), PHP_EOL;
$d = new DateTime('2008-01-01 12:00:00+0200');
$d->setTimeZone($tz2);
echo $d->format(DATE_ISO8601), PHP_EOL; 
?>