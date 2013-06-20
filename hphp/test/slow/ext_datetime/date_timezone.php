<?php

date_default_timezone_set("America/Los_Angeles");

var_dump(date_default_timezone_get());

var_dump(date_default_timezone_set("Asia/Shanghai"));
var_dump(date_default_timezone_get());
var_dump(date_default_timezone_set("America/Los_Angeles"));
var_dump(date_default_timezone_get());

$dt = date_create("2008-08-08 12:34:56");
var_dump(timezone_name_get(date_timezone_get($dt)));

$dt = date_create("2008-08-08 12:34:56");
date_timezone_set($dt, timezone_open("Asia/Shanghai"));
var_dump(timezone_name_get(date_timezone_get($dt)));
var_dump(date_format($dt, 'Y-m-d H:i:s'));

var_dump(timezone_name_from_abbr("CET"));
var_dump(timezone_name_from_abbr("", 3600, 0));

$tz = timezone_open("Asia/Shanghai");
var_dump(timezone_name_get($tz));

// Create two timezone objects, one for Taipei (Taiwan) and one for
// Tokyo (Japan)
$dateTimeZoneTaipei = timezone_open("Asia/Taipei");
$dateTimeZoneJapan = timezone_open("Asia/Tokyo");

// Create two DateTime objects that will contain the same Unix timestamp, but
// have different timezones attached to them.
$dateTimeTaipei = date_create("2008-08-08", $dateTimeZoneTaipei);
$dateTimeJapan = date_create("2008-08-08", $dateTimeZoneJapan);

var_dump(date_offset_get($dateTimeTaipei));
var_dump(date_offset_get($dateTimeJapan));

$tz = timezone_open("Asia/Shanghai");
var_dump(timezone_name_get($tz));

$timezone = timezone_open("CET");
$transitions = timezone_transitions_get($timezone);
var_dump($transitions[0]['ts']);
var_dump($transitions[0]['offset']);
var_dump($transitions[0]['isdst']);
var_dump($transitions[0]['abbr']);

var_dump((bool)timezone_version_get());

