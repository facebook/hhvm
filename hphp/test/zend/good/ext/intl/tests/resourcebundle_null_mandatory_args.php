<?php
ini_set("intl.error_level", E_WARNING);

$r = new ResourceBundle('en_US', NULL);
$c = $r->get('calendar')->get('gregorian')->get('DateTimePatterns')->get(0);
var_dump($c);

ini_set('intl.default_locale', 'pt_PT');
$r = new ResourceBundle(NULL, NULL);
$c = $r->get('calendar')->get('gregorian')->get('DateTimePatterns')->get(0);
var_dump($c);
?>
==DONE==