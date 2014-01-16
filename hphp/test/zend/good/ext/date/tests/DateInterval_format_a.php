<?php

$date1 = new DateTime('2000-01-01 00:00:00');
$date2 = new DateTime('2001-03-04 04:05:06');

$interval = $date1->diff($date2);

echo $interval->format('a=%a') . "\n";

?>