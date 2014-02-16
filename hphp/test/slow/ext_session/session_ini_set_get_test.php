<?php
var_dump(ini_get('session.serialize_handler'));
var_dump(ini_set('session.serialize_handler','php'));
var_dump(ini_get('session.serialize_handler'));

var_dump(ini_get('session.gc_maxlifetime'));
var_dump(ini_set('session.gc_maxlifetime', 123));
var_dump(ini_get('session.gc_maxlifetime'));


