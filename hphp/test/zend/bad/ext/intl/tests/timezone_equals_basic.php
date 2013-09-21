<?php
ini_set("intl.error_level", E_WARNING);

$tz1 = intltz_create_time_zone('Europe/Lisbon');
$tz2 = intltz_create_time_zone('Europe/Lisbon');
echo "Comparison to self:\n";
var_dump($tz1 == $tz1);
echo "Comparison to equal instance:\n";
var_dump($tz1 == $tz2);
echo "Comparison to equivalent instance:\n";
var_dump($tz1 == intltz_create_time_zone('Portugal'));
echo "Comparison to GMT:\n";
var_dump($tz1 == intltz_get_gmt());

?>
==DONE==