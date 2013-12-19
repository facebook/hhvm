<?php

$date = new DateTime("28-July-2008");
$other = new DateTime("31-July-2008");

$diff = date_diff($date, $other);

var_dump($diff);