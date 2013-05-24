<?php

$a = array("\xe8\xaf\xb6", "\xe6\xaf\x94", "\xe8\xa5\xbf");
asort($a);
var_dump($a);

$a = array("\xe8\xaf\xb6", "\xe6\xaf\x94", "\xe8\xa5\xbf");
asort($a, SORT_LOCALE_STRING);
var_dump($a);

$a = array("\xe8\xaf\xb6", "\xe6\xaf\x94", "\xe8\xa5\xbf");
var_dump(setlocale(LC_ALL, 'zh_CN.utf8'));
asort($a );
var_dump($a);

$a = array("\xe8\xaf\xb6", "\xe6\xaf\x94", "\xe8\xa5\xbf");
var_dump(setlocale(LC_ALL, 'zh_CN.utf8'));
asort($a, SORT_LOCALE_STRING);
var_dump($a);
