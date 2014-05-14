<?php

$str = "Holy fuck. Are they actually going to get there?";

$ret = sncompress($str);
$ret = sncompress($ret);
$ret = sncompress($ret);
$ret = snuncompress($ret);
$ret = snuncompress($ret);
$ret = snuncompress($ret);
var_dump($ret === $str);
$str = str_repeat("x", 1000);
$ret = sncompress($str);
$ret = snuncompress($ret);
var_dump($ret === $str);
