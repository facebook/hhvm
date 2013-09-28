<?php

$str = "Holy fuck. Are they actually going to get there?";

$ret = bzcompress($str);
$ret = bzcompress($ret);
$ret = bzcompress($ret);
$ret = bzdecompress($ret);
$ret = bzdecompress($ret);
$ret = bzdecompress($ret);
var_dump($ret === $str);
$str = str_repeat("x", 1000);
$ret = bzcompress($str);
$ret = bzdecompress($ret);
var_dump($ret === $str);
