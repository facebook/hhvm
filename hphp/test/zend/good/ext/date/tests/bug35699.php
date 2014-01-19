<?php
date_default_timezone_set("UTC");

echo date(DATE_ISO8601, strtotime('1964-06-06')), "\n";
echo date(DATE_ISO8601, strtotime('1963-06-06')), "\n";
echo date(DATE_ISO8601, strtotime('1964-01-06')), "\n";
?>