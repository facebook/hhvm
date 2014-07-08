<?php
ini_set("intl.error_level", E_WARNING);
//ini_set("intl.default_locale", "nl");

$fmt = <<<EOD
{0,date} {0,time}
EOD;

$dt = new DateTime("2012-05-06 18:00:42", new DateTimeZone("Europe/Lisbon"));

$mf = new MessageFormatter('en_US', $fmt);

var_dump($mf->format(array($dt)));

?>
==DONE==