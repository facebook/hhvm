<?php
date_default_timezone_set('UTC');

$date = new DateTime('-1500-01-01');
var_dump($date->format('r'));

$date->setDate(-2147483648, 1, 1);
var_dump($date->format('r'));
var_dump($date->format('c'));