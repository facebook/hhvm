<?php

var_dump(ini_set('open_basedir', '/tmp'));
var_dump(ini_set('open_basedir', '/home;/tmp;/invalid_root_dir_asdfasdf/dfg;dfg'));
var_dump(ini_get('open_basedir'));

// Can't add since it isn't more specific
var_dump(ini_set('open_basedir', '/home'));
var_dump(ini_get('open_basedir'));

// Can add
var_dump(ini_set('open_basedir', '/tmp/aasdfasdf'));
var_dump(ini_get('open_basedir'));
