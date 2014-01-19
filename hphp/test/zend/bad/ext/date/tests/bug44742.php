<?php
date_default_timezone_set('Europe/London');
$dates = array(
	"2008-04-11 00:00:00+0000",
	"2008-04-11 00:00:00+0200",
	"2008-04-11 00:00:00+0330",
	"2008-04-11 00:00:00-0500",
	"2008-04-11 00:00:00-1130",
	"2008-04-11 00:00:00 CEST",
	"2008-04-11 00:00:00 CET",
	"2008-04-11 00:00:00 UTC",
	"2008-04-11 00:00:00 America/New_York",
	"2008-04-11 00:00:00 Europe/Oslo",
	"2008-04-11 00:00:00 Asia/Singapore",
);
foreach ($dates as $date)
{
	$date = date_create($date);
	var_dump(timezone_offset_get(date_timezone_get($date), $date));
}
?>