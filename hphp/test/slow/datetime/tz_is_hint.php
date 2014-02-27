<?php

date_default_timezone_set('Asia/Jerusalem');

$format = "c\nO\nZ\nr";
// Local time
var_dump((new DateTime('2014-01-01 00:00'))->format($format));

// The second parameter to the constructor is a hint in how to interpret
// the first, if it can't figure it out. It should *not* change the timezone
// if the input implies it.
$tz = new DateTimeZone('Pacific/Tahiti');
// Tahiti time
var_dump((new DateTime('2014-01-01 00:00', $tz))->format($format));
// UTC
var_dump((new DateTime('@0', $tz))->format($format));
// Pacific-ish
var_dump((new DateTime('2014-01-01 00:00 -08:00', $tz))->format($format));
