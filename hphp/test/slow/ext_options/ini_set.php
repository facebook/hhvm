<?php

// PHP_INI_ALL
var_dump(ini_set('arg_separator.output', 'foo'));
var_dump(ini_get('arg_separator.output'));

// PHP_INI_NONE
var_dump(ini_set('hphp.compiler_id', 'foo'));
var_dump(ini_get('hphp.compiler_id'));

// PHP_INI_SYSTEM
var_dump(ini_set('file_uploads', '0'));
var_dump(ini_get('file_uploads'));

// PHP_INI_ONLY
var_dump(ini_set('expose_php', '0'));
var_dump(ini_get('expose_php'));

// PHP_INI_PERDIR
var_dump(ini_set('always_populate_raw_post_data', '0'));
var_dump(ini_get('always_populate_raw_post_data'));

// ini_set with values expecting numbers but given an empty string
var_dump(ini_set('error_reporting', ''));
var_dump(ini_get('error_reporting'));
