<?php
date_default_timezone_set("Europe/Lisbon");
$a = new DateTime("2009-01-01", null);
echo $a->format(DateTime::COOKIE);
echo "\n";
$a = date_create("2009-01-01", null);
echo $a->format(DateTime::COOKIE);
echo "\n";
?>