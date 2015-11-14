<?php
date_default_timezone_set("Europe/Berlin");

$datetime = new DateTime('2015-01-08');
var_dump($datetime->modify('invalid string'));
echo $datetime->format('Y-m-d');
