<?php
date_default_timezone_set("UTC");

echo date(DATE_ISO8601, strtotime("11/20/2005 8:00 AM \r\n")) . "\n";
echo date(DATE_ISO8601, strtotime("  11/20/2005 8:00 AM \r\n")) . "\n";
var_dump(date_parse(" a "));
var_dump(date_parse(" \n "));
?>