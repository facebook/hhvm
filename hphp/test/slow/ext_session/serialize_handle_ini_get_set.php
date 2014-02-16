<?php
var_dump(ini_get('session.serialize_handler'));
var_dump(ini_set('session.serialize_handler','php'));
var_dump(ini_get('session.serialize_handler'));


