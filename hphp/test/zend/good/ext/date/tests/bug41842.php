<?php
date_default_timezone_set("UTC");

$date = new DateTime('-2007-06-28 00:00:00');
echo $date->format(DATE_ISO8601);
?>