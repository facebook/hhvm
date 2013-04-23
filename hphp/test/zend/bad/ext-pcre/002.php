<?php

var_dump(preg_match());
var_dump(preg_match_all());
var_dump(preg_match_all('//', '', $dummy, 0xdead));

var_dump(preg_quote());
var_dump(preg_quote(''));

var_dump(preg_replace('/(.)/', '${1}${1', 'abc'));
var_dump(preg_replace('/.++\d*+[/', 'for ($', 'abc'));
var_dump(preg_replace('/(.)/e', 'for ($', 'abc'));

?>