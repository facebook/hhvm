<?php
//$debug = true;
ini_set('include_path', dirname(__FILE__));
include_once('common.inc');

// Note: It does not return TRUE/FALSE for setting char

var_dump(mb_substitute_character(0x3044));
var_dump(mb_substitute_character());
var_dump(bin2hex(mb_convert_encoding("\xe2\x99\xa0\xe3\x81\x82", "CP932", "UTF-8")));

var_dump(mb_substitute_character('long'));
var_dump(mb_substitute_character());
var_dump(bin2hex(mb_convert_encoding("\xe2\x99\xa0\xe3\x81\x82", "CP932", "UTF-8")));

var_dump(mb_substitute_character('none'));
var_dump(mb_substitute_character());
var_dump(bin2hex(mb_convert_encoding("\xe2\x99\xa0\xe3\x81\x82", "CP932", "UTF-8")));

var_dump(mb_substitute_character('entity'));
var_dump(mb_substitute_character());
var_dump(bin2hex(mb_convert_encoding("\xe2\x99\xa0\xe3\x81\x82", "CP932", "UTF-8")));

var_dump(mb_substitute_character('BAD_NAME'));
?>