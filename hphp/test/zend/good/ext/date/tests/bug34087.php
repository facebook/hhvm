<?php
date_default_timezone_set("UTC");
echo "Y/m/d: ", strtotime("2005/8/12"), "\n";
echo "Y-m-d: ", strtotime("2005-8-12"), "\n";

echo date(DATE_ISO8601, strtotime("2005/1/2")), "\n";
echo date(DATE_ISO8601, strtotime("2005/01/02")), "\n";
echo date(DATE_ISO8601, strtotime("2005/01/2")), "\n";
echo date(DATE_ISO8601, strtotime("2005/1/02")), "\n";
?>