<?php
class MyDateTime extends DateTime {
	public function __construct() { }
}
class MyDateTimeZone extends DateTimeZone {
	public function __construct() { }
}

$o = new MyDateTime;
var_dump($o->format("d"));
$x = clone $o;

var_dump($x->format("d"));

clone $o;


var_dump(timezone_location_get(clone new MyDateTimezone));
?>