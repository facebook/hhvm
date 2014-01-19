<?php
$x = new DateInterval("P3Y6M4DT12H30M5S");
print_r($x);
$y = unserialize(serialize($x));
print_r($y);