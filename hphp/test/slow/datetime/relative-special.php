<?php
date_default_timezone_set('America/Los_Angeles');
$date = new DateTime('2013-12-27');
var_dump($date->modify('+1 weekday'));
