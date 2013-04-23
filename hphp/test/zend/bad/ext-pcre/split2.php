<?php

var_dump(preg_split('/(\d*)/', 'ab2c3u', -1, PREG_SPLIT_DELIM_CAPTURE));
var_dump(preg_split('/(\d*)/', 'ab2c3u', -1, PREG_SPLIT_OFFSET_CAPTURE));
var_dump(preg_split('/(\d*)/', 'ab2c3u', -1, PREG_SPLIT_NO_EMPTY | PREG_SPLIT_DELIM_CAPTURE));
var_dump(preg_split('/(\d*)/', 'ab2c3u', -1, PREG_SPLIT_NO_EMPTY | PREG_SPLIT_OFFSET_CAPTURE));;
var_dump(preg_split('/(\d*)/', 'ab2c3u', -1, PREG_SPLIT_DELIM_CAPTURE | PREG_SPLIT_OFFSET_CAPTURE));
var_dump(preg_split('/(\d*)/', 'ab2c3u', -1, PREG_SPLIT_NO_EMPTY | PREG_SPLIT_DELIM_CAPTURE | PREG_SPLIT_OFFSET_CAPTURE));


var_dump(preg_last_error(1));
ini_set('pcre.recursion_limit', 1);
var_dump(preg_last_error() == PREG_NO_ERROR);
var_dump(preg_split('/(\d*)/', 'ab2c3u'));
var_dump(preg_last_error() == PREG_RECURSION_LIMIT_ERROR);

?>