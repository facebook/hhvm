<?php

$time_current_timezone = new DateTime('');
$time_current_timezone->setTimestamp(1234567890);

$time_null_timezone = new DateTime('', null);
$time_null_timezone->setTimestamp(1234567890);

var_dump($time_current_timezone < $time_null_timezone);
var_dump($time_current_timezone == $time_null_timezone);
var_dump($time_current_timezone > $time_null_timezone);
