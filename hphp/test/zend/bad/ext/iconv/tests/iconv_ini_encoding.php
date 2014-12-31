<?php
echo "Getting INI\n";
var_dump(ini_get('default_charset'));
var_dump(ini_get('internal_encoding'));
var_dump(ini_get('input_encoding'));
var_dump(ini_get('output_encoding'));

var_dump(ini_get('iconv.internal_encoding'));
var_dump(ini_get('iconv.input_encoding'));
var_dump(ini_get('iconv.output_encoding'));

echo "Setting INI\n";
var_dump(ini_set('default_charset', 'UTF-8'));
var_dump(ini_set('internal_encoding', 'UTF-8'));
var_dump(ini_set('input_encoding', 'UTF-8'));
var_dump(ini_set('output_encoding', 'UTF-8'));
var_dump(ini_set('iconv.internal_encoding', 'UTF-8'));
var_dump(ini_set('iconv.input_encoding', 'UTF-8'));
var_dump(ini_set('iconv.output_encoding', 'UTF-8'));

echo "Getting INI\n";
var_dump(ini_get('default_charset'));
var_dump(ini_get('internal_encoding'));
var_dump(ini_get('input_encoding'));
var_dump(ini_get('output_encoding'));

var_dump(ini_get('iconv.internal_encoding'));
var_dump(ini_get('iconv.input_encoding'));
var_dump(ini_get('iconv.output_encoding'));

