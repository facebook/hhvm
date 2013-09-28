<?php

date_default_timezone_set("America/Los_Angeles");

var_dump(strtotime("now") > 0);
var_dump(strtotime("10 September 2000"));
var_dump(strtotime("+1 day", 968569200));
var_dump(strtotime("+1 week", 968569200));
var_dump(strtotime("+1 week 2 days 4 hours 2 seconds", 968569200));
var_dump(strtotime("next Thursday", 968569200));
var_dump(strtotime("last Monday", 968569200));

$str = "Not Good";
$timestamp = strtotime($str);
var_dump($timestamp);
var_dump(strtotime(""));

