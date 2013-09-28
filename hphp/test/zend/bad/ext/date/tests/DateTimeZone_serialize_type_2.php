<?php
//Set the default time zone 
date_default_timezone_set("Europe/London");

$tz1 = new DateTimeZone("EST");
var_dump( $tz1 );
$serialized = serialize($tz1);
var_dump($serialized); 

$tz2 = unserialize($serialized);
var_dump($tz2);
// Try to use unserialzied object 
var_dump( $tz2->getName() ); 

?>
===DONE=== 