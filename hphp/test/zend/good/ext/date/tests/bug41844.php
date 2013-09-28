<?php
date_default_timezone_set("UTC");

$date = new DateTime('2007-06-28');
$date->modify('-3006 years');
echo $date->format(DATE_ISO8601), "\n";

$date = new DateTime('2007-06-28');
$date->modify('-2008 years');
echo $date->format(DATE_ISO8601), "\n";
?>