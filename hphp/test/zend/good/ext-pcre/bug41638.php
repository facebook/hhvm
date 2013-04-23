<?php
$str = "repeater id='loopt' dataSrc=subject colums=2";

preg_match_all("/(['\"])((.*(\\\\\\1)*)*)\\1/sU",$str,$str_instead);
print_r($str_instead);

// these two are from Magnus Holmgren (extracted from a pcre-dev mailing list post)
preg_match_all("/(['\"])((?:\\\\\\1|.)*)\\1/sU", $str, $str_instead);
print_r($str_instead);

preg_match_all("/(['\"])(.*)(?<!\\\\)\\1/sU", $str, $str_instead);
print_r($str_instead);

?>