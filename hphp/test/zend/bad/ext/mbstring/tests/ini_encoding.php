<?php
echo "Getting INI\n";
var_dump(ini_get('default_charset'));
var_dump(ini_get('internal_encoding'));
var_dump(ini_get('input_encoding'));
var_dump(ini_get('output_encoding'));

var_dump(ini_get('mbstring.internal_encoding'));
var_dump(mb_internal_encoding());
var_dump(ini_get('mbstring.http_input'));
var_dump(ini_get('mbstring.http_output'));

echo "Setting INI\n";
var_dump(ini_set('default_charset', 'UTF-8'));
var_dump(ini_set('internal_encoding', 'UTF-8'));
var_dump(ini_set('input_encoding', 'UTF-8'));
var_dump(ini_set('output_encoding', 'UTF-8'));
var_dump(ini_set('mbstring.internal_encoding', 'UTF-8'));
var_dump(ini_set('mbstring.http_input', 'UTF-8'));
var_dump(ini_set('mbstring.http_output', 'UTF-8'));

echo "Getting INI\n";
var_dump(ini_get('default_charset'));
var_dump(ini_get('internal_encoding'));
var_dump(ini_get('input_encoding'));
var_dump(ini_get('output_encoding'));

var_dump(ini_get('mbstring.internal_encoding'));
var_dump(mb_internal_encoding());
var_dump(ini_get('mbstring.http_input'));
var_dump(ini_get('mbstring.http_output'));


