<?php
var_dump(ini_get('session.serialize_handler'));
var_dump(ini_set('session.serialize_handler','php_binary'));
var_dump(ini_get('session.serialize_handler'));

//try to set invalid serialize_handler
var_dump(ini_set('session.serialize_handler','abcdef'));
var_dump(ini_get('session.serialize_handler'));

var_dump(ini_get('session.gc_maxlifetime'));
var_dump(ini_set('session.gc_maxlifetime', 123));
var_dump(ini_get('session.gc_maxlifetime'));
