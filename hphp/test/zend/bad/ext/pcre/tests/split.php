<?php

var_dump(preg_split());
var_dump(preg_split('/*/', 'x'));

var_dump(preg_split('/[\s, ]+/', 'x yy,zzz'));
var_dump(preg_split('/[\s, ]+/', 'x yy,zzz', -1));
var_dump(preg_split('/[\s, ]+/', 'x yy,zzz', 0));
var_dump(preg_split('/[\s, ]+/', 'x yy,zzz', 1));
var_dump(preg_split('/[\s, ]+/', 'x yy,zzz', 2));

var_dump(preg_split('/\d*/', 'ab2c3u'));
var_dump(preg_split('/\d*/', 'ab2c3u', -1, PREG_SPLIT_NO_EMPTY));

?>