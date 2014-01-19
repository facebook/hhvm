<?php
$b = new ResourceBundle('de_DE', 'ICUDATA-region');
var_dump($b->get('Countries')->get('DE'));

$b = new ResourceBundle('icuver', 'ICUDATA');
var_dump($b->get('ICUVersion') !== NULL);

$b = new ResourceBundle('supplementalData', 'ICUDATA', false);
var_dump($b->get('cldrVersion') !== NULL);
?>