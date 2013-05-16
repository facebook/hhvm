<?php

$ini = parse_ini_string('ini="ini;raw"', null, INI_SCANNER_RAW);
var_dump($ini['ini']);
$ini = parse_ini_string('ini="ini;raw', null, INI_SCANNER_RAW);
var_dump($ini['ini']);
$ini = parse_ini_string('ini=ini;raw', null, INI_SCANNER_RAW);
var_dump($ini['ini']);
$ini = parse_ini_string('ini=ini"raw', null, INI_SCANNER_RAW);
var_dump($ini['ini']);
$ini = parse_ini_string("ini=\r\niniraw", null, INI_SCANNER_RAW);
var_dump($ini['ini']);