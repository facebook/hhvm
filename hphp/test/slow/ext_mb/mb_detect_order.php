<?php

$str = "Pr\xC3\x9C\xC3\x9D"."fung";

/* Set detection order by enumerated list */
mb_detect_order("eucjp-win,sjis-win,UTF-8");
var_dump(mb_detect_encoding($str));
mb_detect_order("eucjp-win,UTF-8,sjis-win");
var_dump(mb_detect_encoding($str));

/* Set detection order by array */
mb_detect_order(array("eucjp-win", "sjis-win", "UTF-8"));
var_dump(mb_detect_encoding($str));
mb_detect_order(array("eucjp-win", "UTF-8", "sjis-win"));
var_dump(mb_detect_encoding($str));

/* Display current detection order */
var_dump(implode(", ", mb_detect_order()));

var_dump(!empty(mb_get_info()['detect_order']));
