<?php
date_default_timezone_set("UTC");

echo date(DATE_ISO8601, strtotime('5 january 2006+3day+1day')) . "\n";

?>