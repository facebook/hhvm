<?php

$datetime = date_create("2010-08-16");
$interval = date_interval_create_from_date_string("2 weeks");
$dt2 = date_add($datetime, $interval);
var_dump(date_format($dt2, "Y-m-d H:i:s") === "2010-08-30 00:00:00");
