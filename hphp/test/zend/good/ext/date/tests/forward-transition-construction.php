<?php
date_default_timezone_set('America/New_York');

$date = new DateTime('2010-03-14 01:30:00');
echo $date->format('Y-m-d H:i:s T/e - U') . "\n";

$date = new DateTime('2010-03-14 02:00:00');
echo $date->format('Y-m-d H:i:s T/e - U') . "\n";

$date = new DateTime('2010-03-14 02:30:00');
echo $date->format('Y-m-d H:i:s T/e - U') . "\n";

$date = new DateTime('2010-03-14 03:00:00');
echo $date->format('Y-m-d H:i:s T/e - U') . "\n";

$date = new DateTime('2010-03-14 03:30:00');
echo $date->format('Y-m-d H:i:s T/e - U') . "\n";
?>
