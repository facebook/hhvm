<?php
ini_set("intl.error_level", E_WARNING);

class A extends IntlTimeZone {
function __construct() {}
}

$tz = new A();
$tz2 = intltz_get_gmt();
var_dump($tz, $tz2);
try {
var_dump($tz == $tz2);
} catch (Exception $e) {
	var_dump(get_class($e), $e->getMessage());
}

?>
==DONE==