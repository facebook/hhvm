<?php
date_default_timezone_set("UTC");

echo date(DATE_ISO8601, strtotime("Sat 26th Nov 2005 18:18")) . "\n";
echo date(DATE_ISO8601, strtotime("26th Nov", 1134340285)) . "\n";
echo date(DATE_ISO8601, strtotime("Dec. 4th, 2005")) . "\n";
echo date(DATE_ISO8601, strtotime("December 4th, 2005")) . "\n";
?>