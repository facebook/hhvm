<?php
$start    = new DateTime('20101212');
$interval = DateInterval::createFromDateString('next day');
$dp = new DatePeriod($start, $interval, 1);
foreach($dp as $dt) {
    echo $dt->format('r') . "\n"; // Sun, 12 Dec 2010 00:00:00 +0100
}
echo $start->format('r'), "\n";
foreach($dp as $dt) {
    echo $dt->format('r') . "\n"; // Sun, 12 Dec 2010 00:00:00 +0100
}
echo $start->format('r'), "\n\n";
?>