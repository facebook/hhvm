<?php
date_default_timezone_set('Asia/Tokyo');

$now = new DateTime('2008-10-10 01:02:03');
echo $now->format("Y-m-d H:i:s") . PHP_EOL;

$now->modify("1 day");
echo $now->format("Y-m-d H:i:s") . PHP_EOL;

$now->modify("1 hour");
echo $now->format("Y-m-d H:i:s") . PHP_EOL;

$now->setTime(0, 0, 0);
//date_time_set($now, 0, 0, 0);
echo $now->format("Y-m-d H:i:s") . PHP_EOL;