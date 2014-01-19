<?php
date_default_timezone_set("UTC");

$t = 1133216119;

echo date(DATE_ISO8601, strtotime("+ 1 day", $t)) . "\n";
echo date(DATE_ISO8601, strtotime("+ 1 month", $t)) . "\n";
echo date(DATE_ISO8601, strtotime("+ 1 week", $t)) . "\n";
?>