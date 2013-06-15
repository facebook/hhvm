<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; }
}

var_dump(date_default_timezone_get());

var_dump(date_default_timezone_set("Asia/Shanghai"));
var_dump(date_default_timezone_get() === "Asia/Shanghai");
var_dump(date_default_timezone_set("America/Los_Angeles"));
var_dump(date_default_timezone_get() === "America/Los_Angeles");

$dt = date_create("2008-08-08 12:34:56");
var_dump(timezone_name_get(date_timezone_get($dt)) === "America/Los_Angeles");

$dt = date_create("2008-08-08 12:34:56");
date_timezone_set($dt, timezone_open("Asia/Shanghai"));
var_dump(timezone_name_get(date_timezone_get($dt)) === "Asia/Shanghai");
var_dump(date_format($dt, 'Y-m-d H:i:s'));

VS(timezone_name_from_abbr("CET"), "Europe/Berlin");
VS(timezone_name_from_abbr("", 3600, 0), "Europe/Paris");

$tz = timezone_open("Asia/Shanghai");
VS(timezone_name_get($tz), "Asia/Shanghai");

// Create two timezone objects, one for Taipei (Taiwan) and one for
// Tokyo (Japan)
$dateTimeZoneTaipei = timezone_open("Asia/Taipei");
$dateTimeZoneJapan = timezone_open("Asia/Tokyo");

// Create two DateTime objects that will contain the same Unix timestamp, but
// have different timezones attached to them.
$dateTimeTaipei = date_create("2008-08-08", $dateTimeZoneTaipei);
$dateTimeJapan = date_create("2008-08-08", $dateTimeZoneJapan);

VS(date_offset_get($dateTimeTaipei), 28800);
VS(date_offset_get($dateTimeJapan), 32400);

$tz = timezone_open("Asia/Shanghai");
VS(timezone_name_get($tz), "Asia/Shanghai");

$timezone = timezone_open("CET");
$transitions = timezone_transitions_get($timezone);
VS($transitions[0]['ts'], -1693706400);
VS($transitions[0]['offset'], 7200);
VS($transitions[0]['isdst'], true);
VS($transitions[0]['abbr'], "CEST");

VS((bool)timezone_version_get(), true);

