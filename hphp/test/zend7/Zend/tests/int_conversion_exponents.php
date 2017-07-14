<?php

var_dump((int)"1.2345e9");
var_dump((int)"-1.2345e9");
var_dump(intval("1.2345e9"));
var_dump(intval("-1.2345e9"));
var_dump("1.2345e9" % PHP_INT_MAX);
var_dump("-1.2345e9" % PHP_INT_MIN);
var_dump("1.2345e9" | 0);
var_dump("-1.2345e9" | 0);

echo PHP_EOL;

var_dump((int)" 1.2345e9  abc");
var_dump((int)" -1.2345e9  abc");
var_dump(intval(" 1.2345e9  abc"));
var_dump(intval(" -1.2345e9  abc"));
var_dump(" 1.2345e9  abc" % PHP_INT_MAX);
var_dump(" -1.2345e9  abc" % PHP_INT_MIN);
var_dump(" 1.2345e9  abc" | 0);
var_dump(" -1.2345e9  abc" | 0);

?>
