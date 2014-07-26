<?php

$str = "Holy fuck. Are they actually going to get there?";

$ret = snappy_compress($str);
$ret = snappy_compress($ret);
$ret = snappy_compress($ret);
$ret = snappy_uncompress($ret);
$ret = snappy_uncompress($ret);
$ret = snappy_uncompress($ret);
var_dump($ret === $str);
$str = str_repeat("x", 1000);
$ret = snappy_compress($str);
$ret = snappy_uncompress($ret);
var_dump($ret === $str);
