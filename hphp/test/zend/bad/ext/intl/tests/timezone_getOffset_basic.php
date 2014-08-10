<?php
ini_set("intl.error_level", E_WARNING);
$ams = IntlTimeZone::createTimeZone('Europe/Amsterdam');

$date = strtotime("1 July 2012 +0000");

var_dump($ams->getOffset($date *1000., true, $rawOffset, $dstOffset),
	$rawOffset, $dstOffset);

$lsb = IntlTimeZone::createTimeZone('Europe/Lisbon');
	
var_dump(intltz_get_offset($lsb, $date *1000., true, $rawOffset, $dstOffset),
	$rawOffset, $dstOffset);

?>
==DONE==
