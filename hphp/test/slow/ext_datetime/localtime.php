<?php

date_default_timezone_set("America/Los_Angeles");

$d = strtotime("2008-09-10 12:34:56");

$localtime = localtime($d);
$localtime_assoc = localtime($d, true);
var_dump($localtime);
var_dump($localtime_assoc);
