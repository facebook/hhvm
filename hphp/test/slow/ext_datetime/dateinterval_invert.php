<?php

date_default_timezone_set("UTC");

$a = new DateTime("@0");
$b = new DateTime("@10");

var_dump($a->diff($b)->invert);
var_dump($b->diff($a)->invert);
