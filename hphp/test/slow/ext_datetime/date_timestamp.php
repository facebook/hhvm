<?php

$tz = timezone_open("America/Los_Angeles");
$dt = date_create("2008-08-08 12:34:56", $tz);
var_dump(date_timestamp_get($dt) === 1218224096);

$tz = timezone_open("America/Los_Angeles");
$dt = date_create("2008-08-08 12:34:56", $tz);
date_timestamp_set($dt, 1000000000);
var_dump(date_format($dt, "Y-m-d H:i:s") === "2001-09-08 18:46:40");

