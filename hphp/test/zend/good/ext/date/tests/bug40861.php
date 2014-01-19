<?php
date_default_timezone_set("GMT");

$offset = +60;
$ts = strtotime('2000-01-01 12:00:00');
$result = date("Y-m-d H:i:s", strtotime("+$offset minutes", $ts));
echo $result . "\n";

$offset = -60;
$ts = strtotime('2000-01-01 12:00:00');
$result = date("Y-m-d H:i:s", strtotime("+$offset minutes", $ts));
echo $result . "\n";

$offset = -60;
$ts = strtotime('2000-01-01 12:00:00');
$result = date("Y-m-d H:i:s", strtotime("-$offset minutes", $ts));
echo $result . "\n";


$offset = 60;
$ts = strtotime('2000-01-01 12:00:00');
$result = date("Y-m-d H:i:s", strtotime("+$offset minutes", $ts));
echo $result . "\n";

?>