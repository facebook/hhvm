<?php
$zones = array(
	"CST6CDT", "Cuba", "Egypt", "Eire", "EST5EDT", "Factory", "GB-Eire",
	"GMT0", "Greenwich", "Hongkong", "Iceland", "Iran", "Israel", "Jamaica",
	"Japan", "Kwajalein", "Libya", "MST7MDT", "Navajo", "NZ-CHAT", "Poland",
	"Portugal", "PST8PDT", "Singapore", "Turkey", "Universal", "W-SU",

	"UTC", "GMT", "GMT+0100", "-0230",
);

foreach ( $zones as $zone )
{
	$d = new DateTimeZone( $zone );
	print_r($d);
}
?>
