<?php
date_default_timezone_set("Europe/London");
$tz = new DateTimeZone("Europe/London");
$tran = $tz->getTransitions();
var_dump( $tran[0] );

$base_time = '28 Feb 2008 12:00:00';
$dt = date_create( "$base_time +10000000000 years" );
echo date_format( $dt, DATE_ISO8601 );
?>