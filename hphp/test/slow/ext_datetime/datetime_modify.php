<?php
date_default_timezone_set('America/Los_Angeles');
$datetime = new DateTime();
$datetime->modify("2014-09-20 20:30 -1 day");
var_dump($datetime->format('c'));
$y = new DateTime("2012-06-23T11:00:00");
$y->modify("+3 days");
var_dump($y);
var_dump($y->format('Y-m-d'));
