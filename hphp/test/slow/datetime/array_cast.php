<?php

date_default_timezone_set('Asia/Jerusalem');
$dt = (new DateTime('2014-01-01 12:34:56'));
var_dump($dt);
var_dump((array) $dt);
