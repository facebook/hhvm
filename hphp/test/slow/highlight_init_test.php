<?php
var_dump(ini_get('highlight.comment'));
var_dump(ini_get('highlight.default'));
var_dump(ini_get('highlight.keyword'));
var_dump(ini_get('highlight.string'));
var_dump(ini_get('highlight.html'));

ini_set('highlight.comment', 'comment');
ini_set('highlight.default', 'default');
ini_set('highlight.keyword', 'keyword');
ini_set('highlight.string', 'string');
ini_set('highlight.html', 'html');

var_dump(ini_get('highlight.comment'));
var_dump(ini_get('highlight.default'));
var_dump(ini_get('highlight.keyword'));
var_dump(ini_get('highlight.string'));
var_dump(ini_get('highlight.html'));
