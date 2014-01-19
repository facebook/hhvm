<?php
date_default_timezone_set("UTC");

$t = 1140973388;

var_dump(strtotime("-2 hours", $t));
var_dump(strtotime("-2\thours", $t));
?>